import bpy
import blf
import gpu

bl_info = {
    "name": "UWB-KITty",
    "blender": (4, 0, 0),
    "category": "Object"
}


# Global draw handle
_draw_handle = None

# Name of the object to track
OBJECT_NAME = "Cube"  # You can change this to any object name


def draw_text(self, context):
    obj = bpy.data.objects.get(OBJECT_NAME)
    if obj:
        x = obj.location.x
        y = obj.location.y
        z = obj.location.z
        blf.size(0, 20)
        blf.position(0, 50, 50, 0)
        blf.draw(0, f"X Position: {x:.2f}")

        blf.position(0, 50, 80, 0)
        blf.draw(0, f"Y Position: {y:.2f}")
        
        blf.position(0, 50, 110, 0)
        blf.draw(0, f"Z Position: {z:.2f}")

    else:
        blf.position(0, 50, 50, 0)
        blf.size(0, 20)
        blf.draw(0, f"Object '{OBJECT_NAME}' not found")


class VIEW3D_OT_toggle_tracking(bpy.types.Operator):
    bl_idname = "view3d.toggle_object_tracking"
    bl_label = "Toggle Object Tracking"

    def execute(self, context):
        global _draw_handle

        if _draw_handle is None:
            _draw_handle = bpy.types.SpaceView3D.draw_handler_add(
                draw_text, (self, context), 'WINDOW', 'POST_PIXEL'
            )
            self.report({'INFO'}, "Tracking enabled.")
        else:
            bpy.types.SpaceView3D.draw_handler_remove(_draw_handle, 'WINDOW')
            _draw_handle = None
            self.report({'INFO'}, "Tracking disabled.")

        context.area.tag_redraw()
        return {'FINISHED'}


class VIEW3D_PT_tracking_panel(bpy.types.Panel):
    bl_label = "Object Tracker"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'UWB-KITty'

    def draw(self, context):
        self.layout.label(text=f"Tracking: '{OBJECT_NAME}'")
        self.layout.operator("view3d.toggle_object_tracking", text="Toggle Tracking")


def register():
    bpy.utils.register_class(VIEW3D_OT_toggle_tracking)
    bpy.utils.register_class(VIEW3D_PT_tracking_panel)


def unregister():
    bpy.utils.unregister_class(VIEW3D_OT_toggle_tracking)
    bpy.utils.unregister_class(VIEW3D_PT_tracking_panel)


if __name__ == "__main__":
    register()
