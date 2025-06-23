import bpy

OBJECT_NAME = "Cube"  # Replace with your object name


class VIEW3D_PT_tracking_panel(bpy.types.Panel):
    bl_label = "Object Tracker"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'UWB-KITty'

    def draw(self, context):
        self.layout.label(text=f"Tracking: '{OBJECT_NAME}'")
        self.layout.operator(
            "view3d.toggle_object_tracking", text="Toggle Tracking")


def register():
    bpy.utils.register_class(VIEW3D_PT_tracking_panel)


def unregister():
    bpy.utils.unregister_class(VIEW3D_PT_tracking_panel)
