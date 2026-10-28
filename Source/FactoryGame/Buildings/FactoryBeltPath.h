// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/*
 *  Bahn-Berechnung für Förderbänder: aus Start, Ende und (optional) den Richtungen an beiden Enden
 *  wird eine Kurve aus Spline-Punkten gebaut. Drei Modi (Wechsel mit R beim Bauen):
 *  - Standard: eine weiche Kurve, die an Ports/Stützen in deren Richtung startet bzw. ankommt
 *  - Gerade:   nur gerade Stücke und 90°-Ecken (Radius 2 m), als L- oder S-Form
 *  - Kurve:    gleichmäßiger Kreisbogen, an einem festen Ende Bogen + Gerade
 *  Höhenunterschiede werden gleichmäßig über die ganze Länge verteilt.
 *  Grenzen (Docs/Standards/13-bauen.md): 0,5–56 m lang, max. 35° Steigung, Kurvenradius min. 2 m.
 */

/** Build mode of a belt */
enum class EFactoryBeltMode : uint8
{
	Standard,
	Straight,
	Curve,

	Count
};

/** One point of a belt path, like a spline point with custom tangents */
struct FFactoryBeltPathPoint
{
	FVector Location = FVector::ZeroVector;
	FVector ArriveTangent = FVector::ZeroVector;
	FVector LeaveTangent = FVector::ZeroVector;
};

/** Why a belt path can't be built */
enum class EFactoryBeltPathProblem : uint8
{
	None,
	TooShort,
	TooLong,
	TooSteep,
	TooTight,

	/** The mode can't connect these two ends, e.g. Straight with a port facing sideways */
	Impossible
};

struct FACTORYGAME_API FFactoryBeltPath
{
	TArray<FFactoryBeltPathPoint> Points;

	/** Horizontal flow direction at the start and at the end */
	FVector StartDirection = FVector::ForwardVector;
	FVector EndDirection = FVector::ForwardVector;

	/** Length along the curve in cm, set by Validate */
	double Length = 0.0;

	/** Returns the position at parameter T (0 - number of segments) */
	FVector Evaluate(double T) const;
};

namespace FactoryBeltPath
{
	/** Limits per belt segment, in cm and degrees */
	constexpr double MinLength = 50.0;
	constexpr double MaxLength = 5600.0;
	constexpr double MaxSlopeDegrees = 35.0;
	constexpr double MinRadius = 200.0;

	/**
	 *  Builds the path from Start to End. StartDirection/EndDirection are the flow directions at the ends
	 *  (only their horizontal part is used); zero means the end is free and the mode chooses.
	 *  Returns Impossible if the mode can't connect the ends, else validates the result
	 */
	FACTORYGAME_API EFactoryBeltPathProblem Build(EFactoryBeltMode Mode, const FVector& Start, const FVector& StartDirection,
		const FVector& End, const FVector& EndDirection, FFactoryBeltPath& OutPath);

	/** Checks length, slope and curve radius and stores the length in the path */
	FACTORYGAME_API EFactoryBeltPathProblem Validate(FFactoryBeltPath& Path);
}
