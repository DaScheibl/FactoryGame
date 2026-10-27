"""FactoryGame – Blender → Unreal Export.

Prüft die Auswahl gegen Docs/Standards/01-naming.md und 06-blender-export.md
und exportiert sie als FBX mit dem Preset ``FactoryGame_UE``.

Installation: Edit → Preferences → Add-ons → Install from Disk → diese Datei.
Panel: 3D-View → Seitenleiste (N) → Tab "FactoryGame".
"""
import re
from pathlib import Path

import bpy

bl_info = {
    "name": "FactoryGame Export",
    "author": "FactoryGame",
    "version": (0, 1, 0),
    "blender": (4, 2, 0),
    "location": "View3D > Sidebar > FactoryGame",
    "description": "Prüft Naming/Skalierung und exportiert FBX für Unreal",
    "category": "Import-Export",
}

CATEGORIES = "BLD|ITM|RES|LOG|STR|VEH|EQP|ENV|UI|SFX|MUS|VFX|MAT"
RE_MESH = re.compile(rf"^(SM|SK)_({CATEGORIES})_[A-Za-z0-9]+(_[A-Za-z0-9]+)*$")
RE_UCX = re.compile(r"^UCX_(?P<mesh>.+)_\d{2}$")
RE_SOCKET = re.compile(r"^SOCKET_[A-Za-z]+(_[A-Za-z0-9]+)*$")
RE_LOD = re.compile(r"_LOD\d$")


def check_objects(objects):
    """Gibt eine Liste von Fehlermeldungen zurück (leer = alles ok)."""
    errors = []
    mesh_names = {o.name for o in objects if o.type in {"MESH", "ARMATURE"}}

    if bpy.context.scene.unit_settings.scale_length != 1.0:
        errors.append("Szene: Unit Scale muss 1.0 sein")

    for obj in objects:
        name = obj.name
        if name.startswith("UCX_"):
            m = RE_UCX.match(name)
            if not m:
                errors.append(f"{name}: Kollision muss UCX_<Mesh>_01 heißen")
            elif m["mesh"] not in mesh_names:
                errors.append(f"{name}: zugehöriges Mesh '{m['mesh']}' nicht in der Auswahl")
        elif name.startswith("SOCKET_"):
            if obj.type != "EMPTY":
                errors.append(f"{name}: Socket muss ein Empty sein")
            elif not RE_SOCKET.match(name):
                errors.append(f"{name}: Socket-Name ungültig (SOCKET_<Typ>_<Nr>)")
            elif obj.parent is None or obj.parent.name not in mesh_names:
                errors.append(f"{name}: Socket muss Child des Render-Meshes sein")
        elif obj.type in {"MESH", "ARMATURE"}:
            base = RE_LOD.sub("", name)
            if not RE_MESH.match(base):
                errors.append(f"{name}: Name muss SM_/SK_ + ID sein, z. B. SM_BLD_Drill_Mk1")
        elif obj.type == "EMPTY":
            # Gruppen-Empty für LODs darf wie das Mesh heißen
            if not RE_MESH.match(name):
                errors.append(f"{name}: unbekanntes Empty (nur SOCKET_ oder LOD-Gruppe)")

        if obj.type == "MESH" and any(abs(s - 1.0) > 1e-4 for s in obj.scale):
            errors.append(f"{name}: Skalierung nicht angewendet (Ctrl+A → Scale)")
        if obj.type == "MESH" and any(abs(r) > 1e-4 for r in obj.rotation_euler):
            errors.append(f"{name}: Rotation nicht angewendet (Ctrl+A → Rotation)")

    if not any(RE_MESH.match(RE_LOD.sub("", n)) for n in mesh_names):
        errors.append("Kein Render-Mesh (SM_/SK_) ausgewählt")
    return errors


def export_name(objects):
    for obj in objects:
        if obj.type in {"MESH", "ARMATURE"} and not obj.name.startswith("UCX_"):
            return RE_LOD.sub("", obj.name)
    return "Export"


class FG_OT_check(bpy.types.Operator):
    bl_idname = "factorygame.check"
    bl_label = "Auswahl prüfen"

    def execute(self, context):
        errors = check_objects(context.selected_objects)
        for e in errors:
            self.report({"WARNING"}, e)
        if not errors:
            self.report({"INFO"}, "Alles in Ordnung")
        return {"FINISHED"}


class FG_OT_export(bpy.types.Operator):
    bl_idname = "factorygame.export"
    bl_label = "FBX exportieren"

    def execute(self, context):
        objects = context.selected_objects
        errors = check_objects(objects)
        if errors:
            for e in errors:
                self.report({"ERROR"}, e)
            return {"CANCELLED"}
        if not bpy.data.filepath:
            self.report({"ERROR"}, ".blend-Datei zuerst speichern")
            return {"CANCELLED"}

        path = Path(bpy.path.abspath("//")) / f"{export_name(objects)}.fbx"
        has_anim = any(o.animation_data for o in objects)
        bpy.ops.export_scene.fbx(
            filepath=str(path),
            use_selection=True,
            object_types={"EMPTY", "ARMATURE", "MESH"},
            apply_scale_options="FBX_SCALE_UNITS",
            axis_forward="-Y",
            axis_up="Z",
            apply_unit_scale=True,
            bake_space_transform=False,
            mesh_smooth_type="FACE",
            use_tspace=True,
            use_mesh_modifiers=True,
            add_leaf_bones=False,
            bake_anim=has_anim,
        )
        self.report({"INFO"}, f"Exportiert: {path}")
        return {"FINISHED"}


class FG_PT_panel(bpy.types.Panel):
    bl_label = "FactoryGame Export"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "FactoryGame"

    def draw(self, context):
        col = self.layout.column()
        col.operator(FG_OT_check.bl_idname, icon="CHECKMARK")
        col.operator(FG_OT_export.bl_idname, icon="EXPORT")


CLASSES = (FG_OT_check, FG_OT_export, FG_PT_panel)


def register():
    for cls in CLASSES:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(CLASSES):
        bpy.utils.unregister_class(cls)


if __name__ == "__main__":
    register()
