# Erzeugt die flache Testwelt /Game/FactoryGame/Maps/Lvl_Flat (zum Testen des Bausystems).
#
# 2 x 2 km große Ebene mit WorldGridMaterial (zeigt das 1-m-Raster), Sonne, Himmel, Nebel und Startpunkt.
# Ausführen ohne geöffneten Editor (Editor vorher schließen):
#   UnrealEditor-Cmd.exe "<Pfad>\FactoryGame.uproject" -run=pythonscript -script="<Pfad>\Tools\CreateFlatMap.py" -unattended
# Eine bestehende Lvl_Flat wird überschrieben.

import unreal

MAP_PATH = "/Game/FactoryGame/Maps/Lvl_Flat"
FLOOR_SIZE_M = 2000

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
    unreal.EditorAssetLibrary.delete_asset(MAP_PATH)

if not level_editor.new_level(MAP_PATH):
    raise RuntimeError("Could not create " + MAP_PATH)


def spawn(actor_class, location, rotation=unreal.Rotator(0, 0, 0), label=None):
    actor = actors.spawn_actor_from_class(actor_class, location, rotation)
    if label:
        actor.set_actor_label(label)
    return actor


# Boden: Engine-Würfel (1 m) auf 2 km gestreckt, Oberkante bei Z = 0
floor = spawn(unreal.StaticMeshActor, unreal.Vector(0, 0, -50), label="Floor")
floor_mesh = floor.static_mesh_component
floor_mesh.set_static_mesh(unreal.load_asset("/Engine/BasicShapes/Cube.Cube"))
floor_mesh.set_material(0, unreal.load_asset("/Engine/EngineMaterials/WorldGridMaterial.WorldGridMaterial"))
floor.set_actor_scale3d(unreal.Vector(FLOOR_SIZE_M, FLOOR_SIZE_M, 1))

# Sonne (bewegliches Licht, keine Licht-Berechnung nötig) und Himmel
sun = spawn(unreal.DirectionalLight, unreal.Vector(0, 0, 1000), unreal.Rotator(roll=0, pitch=-50, yaw=30), "Sun")
sun_light = sun.get_component_by_class(unreal.DirectionalLightComponent)
sun_light.set_mobility(unreal.ComponentMobility.MOVABLE)
sun_light.set_editor_property("atmosphere_sun_light", True)

spawn(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0), label="SkyAtmosphere")

sky = spawn(unreal.SkyLight, unreal.Vector(0, 0, 500), label="SkyLight")
sky_light = sky.get_component_by_class(unreal.SkyLightComponent)
sky_light.set_mobility(unreal.ComponentMobility.MOVABLE)
sky_light.set_editor_property("real_time_capture", True)

spawn(unreal.ExponentialHeightFog, unreal.Vector(0, 0, 0), label="HeightFog")

# Startpunkt in der Mitte, leicht über dem Boden
spawn(unreal.PlayerStart, unreal.Vector(0, 0, 120), label="PlayerStart")

if not level_editor.save_current_level():
    raise RuntimeError("Could not save " + MAP_PATH)

unreal.log("Created flat test map " + MAP_PATH)
