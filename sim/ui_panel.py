import bpy


class VIEW3D_PT_tracking_panel(bpy.types.Panel):
    bl_label = "Object Tracker"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'UWB-KITty'

    def draw(self, context):
        self.layout.label(text=f"Tracking: '{context.active_object.name}'")
        self.layout.operator(
            "view3d.toggle_object_tracking", text="Toggle Tracking")


# Optional: UI Panel
class VIEW3D_PT_distance_panel(bpy.types.Panel):
    bl_label = "Distance Display"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'UWB-KITty'

    def draw(self, context):
        self.layout.operator("view3d.toggle_distance_draw")


def register():
    bpy.utils.register_class(VIEW3D_PT_tracking_panel)
    bpy.utils.register_class(VIEW3D_PT_distance_panel)


def unregister():
    bpy.utils.unregister_class(VIEW3D_PT_tracking_panel)
    bpy.utils.unregister_class(VIEW3D_PT_distance_panel)
