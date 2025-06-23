import bpy
from ..draw.blf_text import draw_text

_draw_handle = None


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


def register():
    bpy.utils.register_class(VIEW3D_OT_toggle_tracking)


def unregister():
    bpy.utils.unregister_class(VIEW3D_OT_toggle_tracking)
    global _draw_handle
    if _draw_handle is not None:
        bpy.types.SpaceView3D.draw_handler_remove(_draw_handle, 'WINDOW')
        _draw_handle = None
