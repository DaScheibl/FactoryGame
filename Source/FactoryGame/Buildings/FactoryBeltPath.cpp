// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryBeltPath.h"

namespace
{
	// Abtastschritte pro Spline-Abschnitt für Länge, Steigung und Radius
	constexpr int32 SamplesPerSegment = 24;

	// Hermite-Tangente für einen Kreisbogen mit Winkel Angle (Bezier-Näherung, Fehler < 0,03 %)
	double ArcTangentLength(double Radius, double Angle)
	{
		return 4.0 * Radius * FMath::Tan(Angle * 0.25);
	}

	FVector Horizontal(const FVector& Vector)
	{
		return FVector(Vector.X, Vector.Y, 0.0).GetSafeNormal();
	}

	FVector RotateAroundUp(const FVector& Vector, double Angle)
	{
		return FQuat(FVector::UpVector, Angle).RotateVector(Vector);
	}

	// vorzeichenbehafteter Winkel von A nach B um die Hochachse
	double SignedAngle(const FVector& A, const FVector& B)
	{
		return FMath::Atan2(FVector::CrossProduct(A, B).Z, FVector::DotProduct(A, B));
	}

	/** Collects path points; a point at the same place as the last one only replaces its leave tangent */
	struct FPathBuilder
	{
		TArray<FFactoryBeltPathPoint> Points;

		void Add(const FVector& Location, const FVector& Arrive, const FVector& Leave)
		{
			if (Points.Num() > 0 && FVector::Dist(Points.Last().Location, Location) < 1.0)
			{
				Points.Last().LeaveTangent = Leave;
				return;
			}

			Points.Add({ Location, Arrive, Leave });
		}

		void AddStraight(const FVector& From, const FVector& To)
		{
			const FVector Tangent = To - From;
			Add(From, Tangent, Tangent);
			Add(To, Tangent, Tangent);
		}

		/** Adds a circular arc. Returns the end point and writes the end direction */
		FVector AddArc(const FVector& From, const FVector& Direction, double TurnSign, double Radius, double Angle, FVector& OutEndDirection)
		{
			const FVector Side = RotateAroundUp(Direction, TurnSign * HALF_PI);
			const int32 NumPieces = FMath::Max(1, FMath::CeilToInt32(Angle / (UE_PI / 4.0)));
			const double PieceAngle = Angle / NumPieces;
			const double TangentLength = ArcTangentLength(Radius, PieceAngle);

			FVector Location = From;

			for (int32 Index = 0; Index <= NumPieces; ++Index)
			{
				const double A = PieceAngle * Index;
				Location = From + Radius * (FMath::Sin(A) * Direction + (1.0 - FMath::Cos(A)) * Side);
				const FVector Tangent = RotateAroundUp(Direction, TurnSign * A) * TangentLength;
				Add(Location, Tangent, Tangent);
			}

			OutEndDirection = RotateAroundUp(Direction, TurnSign * Angle);
			return Location;
		}

		/** Straight lines through the corners, each corner rounded with the minimum radius */
		bool AddPolyline(const TArray<FVector>& Corners)
		{
			const double Radius = FactoryBeltPath::MinRadius;
			FVector Current = Corners[0];

			for (int32 Index = 1; Index < Corners.Num() - 1; ++Index)
			{
				const FVector In = Horizontal(Corners[Index] - Corners[Index - 1]);
				const FVector Out = Horizontal(Corners[Index + 1] - Corners[Index]);
				const FVector ArcStart = Corners[Index] - In * Radius;

				// die Gerade bis zum Bogen darf nicht rückwärts laufen, sonst ist es zu eng
				if (FVector::DotProduct(ArcStart - Current, In) < -1.0 || FVector::Dist(Corners[Index + 1], Corners[Index]) < Radius - 1.0)
				{
					return false;
				}

				AddStraight(Current, ArcStart);

				FVector EndDirection;
				const double Turn = SignedAngle(In, Out);
				Current = AddArc(ArcStart, In, FMath::Sign(Turn), Radius, FMath::Abs(Turn), EndDirection);
			}

			AddStraight(Current, Corners.Last());
			return true;
		}
	};

	// 2x2-Gleichungssystem A*x + B*y = C (nur XY), false wenn A und B parallel sind
	bool Solve2D(const FVector& A, const FVector& B, const FVector& C, double& OutX, double& OutY)
	{
		const double Det = A.X * B.Y - A.Y * B.X;

		if (FMath::Abs(Det) < UE_KINDA_SMALL_NUMBER)
		{
			return false;
		}

		OutX = (C.X * B.Y - C.Y * B.X) / Det;
		OutY = (A.X * C.Y - A.Y * C.X) / Det;
		return true;
	}

	bool BuildStandard(const FVector& Delta, FVector StartDir, FVector EndDir, FPathBuilder& Builder)
	{
		const FVector Chord = Horizontal(Delta);
		const double Distance = FVector(Delta.X, Delta.Y, 0.0).Size();

		// freie Enden: Start entlang der Sehne; Ende gespiegelt zum Start, das ergibt einen gleichmäßigen Bogen
		if (StartDir.IsZero())
		{
			StartDir = Chord;
		}

		if (EndDir.IsZero())
		{
			EndDir = 2.0 * FVector::DotProduct(StartDir, Chord) * Chord - StartDir;
		}

		Builder.Add(FVector::ZeroVector, StartDir * Distance, StartDir * Distance);
		Builder.Add(FVector(Delta.X, Delta.Y, 0.0), EndDir * Distance, EndDir * Distance);
		return true;
	}

	bool BuildStraight(const FVector& Delta, FVector StartDir, const FVector& EndDir, FPathBuilder& Builder)
	{
		const FVector Flat(Delta.X, Delta.Y, 0.0);

		// freier Start: entlang der Welt-Achse, die am besten zum Ziel passt
		if (StartDir.IsZero())
		{
			StartDir = FMath::Abs(Flat.X) >= FMath::Abs(Flat.Y) ? FVector(FMath::Sign(Flat.X), 0.0, 0.0) : FVector(0.0, FMath::Sign(Flat.Y), 0.0);
		}

		const FVector Side = RotateAroundUp(StartDir, HALF_PI);
		const double Forward = FVector::DotProduct(Flat, StartDir);
		const double Lateral = FVector::DotProduct(Flat, Side);
		const FVector LateralDir = Side * FMath::Sign(Lateral);

		if (Forward <= 0.0)
		{
			return false;
		}

		// genau geradeaus
		if (FMath::Abs(Lateral) < 1.0 && (EndDir.IsZero() || FVector::DotProduct(EndDir, StartDir) > 0.99))
		{
			Builder.AddStraight(FVector::ZeroVector, Flat);
			return true;
		}

		// S-Form: Ende zeigt in dieselbe Richtung wie der Start
		if (!EndDir.IsZero() && FVector::DotProduct(EndDir, StartDir) > 0.99)
		{
			const FVector First = StartDir * Forward * 0.5;
			return Builder.AddPolyline({ FVector::ZeroVector, First, First + Side * Lateral, Flat });
		}

		// L-Form: erst geradeaus, dann um 90° zum Ziel; ein festes Ende muss zur zweiten Geraden passen
		if (EndDir.IsZero() || FVector::DotProduct(EndDir, LateralDir) > 0.99)
		{
			return Builder.AddPolyline({ FVector::ZeroVector, StartDir * Forward, Flat });
		}

		return false;
	}

	bool BuildCurve(const FVector& Delta, FVector StartDir, const FVector& EndDir, FPathBuilder& Builder)
	{
		const FVector Flat(Delta.X, Delta.Y, 0.0);

		if (StartDir.IsZero())
		{
			StartDir = Horizontal(Flat);
		}

		const FVector Side = RotateAroundUp(StartDir, HALF_PI);
		const double Forward = FVector::DotProduct(Flat, StartDir);
		const double Lateral = FVector::DotProduct(Flat, Side);
		FVector Ignored;

		// freies Ende: ein einziger Kreisbogen durch das Ziel (höchstens ein Halbkreis)
		if (EndDir.IsZero())
		{
			if (FMath::Abs(Lateral) < 1.0)
			{
				if (Forward <= 0.0)
				{
					return false;
				}

				Builder.AddStraight(FVector::ZeroVector, Flat);
				return true;
			}

			const double Radius = (Forward * Forward + Lateral * Lateral) / (2.0 * FMath::Abs(Lateral));
			const double Angle = 2.0 * FMath::Atan2(FMath::Abs(Lateral), Forward);

			if (Angle > UE_PI + 0.01)
			{
				return false;
			}

			Builder.AddArc(FVector::ZeroVector, StartDir, FMath::Sign(Lateral), Radius, Angle, Ignored);
			return true;
		}

		// festes Ende: Bogen + Gerade oder Gerade + Bogen, der größere passende Radius gewinnt
		const double Turn = SignedAngle(StartDir, EndDir);
		const double Angle = FMath::Abs(Turn);
		const double TurnSign = FMath::Sign(Turn);

		if (Angle < FMath::DegreesToRadians(1.0))
		{
			if (FMath::Abs(Lateral) < 1.0 && Forward > 0.0)
			{
				Builder.AddStraight(FVector::ZeroVector, Flat);
				return true;
			}

			return false;
		}

		const FVector TurnSide = RotateAroundUp(StartDir, TurnSign * HALF_PI);
		const FVector ArcOffset = FMath::Sin(Angle) * StartDir + (1.0 - FMath::Cos(Angle)) * TurnSide;

		double ArcFirstRadius = -1.0, ArcFirstStraight = -1.0;
		double StraightFirstLength = -1.0, StraightFirstRadius = -1.0;
		const bool bArcFirst = Solve2D(ArcOffset, EndDir, Flat, ArcFirstRadius, ArcFirstStraight) && ArcFirstStraight >= 0.0 && ArcFirstRadius > 0.0;
		const bool bStraightFirst = Solve2D(StartDir, ArcOffset, Flat, StraightFirstLength, StraightFirstRadius) && StraightFirstLength >= 0.0 && StraightFirstRadius > 0.0;

		if (bArcFirst && (!bStraightFirst || ArcFirstRadius >= StraightFirstRadius))
		{
			const FVector ArcEnd = Builder.AddArc(FVector::ZeroVector, StartDir, TurnSign, ArcFirstRadius, Angle, Ignored);
			Builder.AddStraight(ArcEnd, Flat);
			return true;
		}

		if (bStraightFirst)
		{
			const FVector ArcStart = StartDir * StraightFirstLength;
			Builder.AddStraight(FVector::ZeroVector, ArcStart);
			Builder.AddArc(ArcStart, StartDir, TurnSign, StraightFirstRadius, Angle, Ignored);
			return true;
		}

		return false;
	}

	double SegmentLength2D(const FFactoryBeltPathPoint& A, const FFactoryBeltPathPoint& B)
	{
		double Length = 0.0;
		FVector Previous = A.Location;

		for (int32 Step = 1; Step <= SamplesPerSegment; ++Step)
		{
			const FVector Point = FMath::CubicInterp(A.Location, A.LeaveTangent, B.Location, B.ArriveTangent, double(Step) / SamplesPerSegment);
			Length += FVector::Dist2D(Previous, Point);
			Previous = Point;
		}

		return Length;
	}
}

FVector FFactoryBeltPath::Evaluate(double T) const
{
	if (Points.Num() < 2)
	{
		return Points.Num() > 0 ? Points[0].Location : FVector::ZeroVector;
	}

	const int32 Segment = FMath::Clamp(FMath::FloorToInt32(T), 0, Points.Num() - 2);

	const FFactoryBeltPathPoint& A = Points[Segment];
	const FFactoryBeltPathPoint& B = Points[Segment + 1];
	return FMath::CubicInterp(A.Location, A.LeaveTangent, B.Location, B.ArriveTangent, FMath::Clamp(T - Segment, 0.0, 1.0));
}

EFactoryBeltPathProblem FactoryBeltPath::Build(EFactoryBeltMode Mode, const FVector& Start, const FVector& StartDirection,
	const FVector& End, const FVector& EndDirection, FFactoryBeltPath& OutPath)
{
	OutPath = FFactoryBeltPath();

	const FVector Delta = End - Start;
	const FVector StartDir = Horizontal(StartDirection);
	const FVector EndDir = Horizontal(EndDirection);

	if (FVector(Delta.X, Delta.Y, 0.0).Size() < MinLength)
	{
		return EFactoryBeltPathProblem::TooShort;
	}

	// erst flach (Z = 0) im Koordinatensystem des Starts bauen, danach die Höhe verteilen
	FPathBuilder Builder;
	bool bBuilt = false;

	switch (Mode)
	{
	case EFactoryBeltMode::Straight:	bBuilt = BuildStraight(Delta, StartDir, EndDir, Builder); break;
	case EFactoryBeltMode::Curve:		bBuilt = BuildCurve(Delta, StartDir, EndDir, Builder); break;
	default:							bBuilt = BuildStandard(Delta, StartDir, EndDir, Builder); break;
	}

	if (!bBuilt || Builder.Points.Num() < 2)
	{
		return EFactoryBeltPathProblem::Impossible;
	}

	TArray<FFactoryBeltPathPoint>& Points = Builder.Points;

	// Höhe gleichmäßig über die waagerechte Länge verteilen: gleiche Steigung auf der ganzen Strecke
	TArray<double> Distances;
	Distances.Add(0.0);

	for (int32 Index = 1; Index < Points.Num(); ++Index)
	{
		Distances.Add(Distances.Last() + SegmentLength2D(Points[Index - 1], Points[Index]));
	}

	const double TotalFlat = FMath::Max(Distances.Last(), 1.0);
	const double Slope = Delta.Z / TotalFlat;

	for (int32 Index = 0; Index < Points.Num(); ++Index)
	{
		FFactoryBeltPathPoint& Point = Points[Index];
		Point.Location = Start + FVector(Point.Location.X, Point.Location.Y, Delta.Z * Distances[Index] / TotalFlat);
		Point.ArriveTangent.Z = Point.ArriveTangent.Size2D() * Slope;
		Point.LeaveTangent.Z = Point.LeaveTangent.Size2D() * Slope;
	}

	OutPath.Points = MoveTemp(Points);
	OutPath.StartDirection = Horizontal(OutPath.Points[0].LeaveTangent);
	OutPath.EndDirection = Horizontal(OutPath.Points.Last().ArriveTangent);

	// ein festes Ende muss auch wirklich in dieser Richtung ankommen
	if (!EndDir.IsZero() && FVector::DotProduct(OutPath.EndDirection, EndDir) < 0.99)
	{
		return EFactoryBeltPathProblem::Impossible;
	}

	return Validate(OutPath);
}

EFactoryBeltPathProblem FactoryBeltPath::Validate(FFactoryBeltPath& Path)
{
	TArray<FVector> Samples;

	for (int32 Segment = 0; Segment + 1 < Path.Points.Num(); ++Segment)
	{
		for (int32 Step = (Segment == 0 ? 0 : 1); Step <= SamplesPerSegment; ++Step)
		{
			Samples.Add(Path.Evaluate(Segment + double(Step) / SamplesPerSegment));
		}
	}

	Path.Length = 0.0;
	EFactoryBeltPathProblem Problem = EFactoryBeltPathProblem::None;
	const double MaxSlope = FMath::Tan(FMath::DegreesToRadians(MaxSlopeDegrees + 0.5));

	for (int32 Index = 1; Index < Samples.Num(); ++Index)
	{
		const FVector Step = Samples[Index] - Samples[Index - 1];
		Path.Length += Step.Size();

		const double Flat = Step.Size2D();

		if (Flat > 1.0 && FMath::Abs(Step.Z) / Flat > MaxSlope)
		{
			Problem = EFactoryBeltPathProblem::TooSteep;
		}

		// Kurvenradius aus drei Punkten (Umkreis), nur waagerecht; 10 % Toleranz für die Bogen-Näherung
		if (Index + 1 < Samples.Num() && Problem == EFactoryBeltPathProblem::None)
		{
			const FVector2D A(Samples[Index - 1]), B(Samples[Index]), C(Samples[Index + 1]);
			const double Area2 = FMath::Abs((B - A) ^ (C - A));

			if (Area2 > 0.01)
			{
				const double Radius = (B - A).Size() * (C - B).Size() * (C - A).Size() / (2.0 * Area2);

				if (Radius < MinRadius * 0.9)
				{
					Problem = EFactoryBeltPathProblem::TooTight;
				}
			}
		}
	}

	if (Path.Length < MinLength)
	{
		return EFactoryBeltPathProblem::TooShort;
	}

	if (Path.Length > MaxLength)
	{
		return EFactoryBeltPathProblem::TooLong;
	}

	return Problem;
}
