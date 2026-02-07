import bpy
from ..draw.distances import draw_distances


class VIEW3D_OT_toggle_distance_draw(bpy.types.Operator):
    bl_idname = "view3d.toggle_distance_draw"
    bl_label = "Toggle Distance Drawing"

    _handle = None

    @classmethod
    def poll(cls, context):
        return context.area.type == 'VIEW_3D'

    def execute(self, context):
        if VIEW3D_OT_toggle_distance_draw._handle is None:
            args = (self, context)
            VIEW3D_OT_toggle_distance_draw._handle = bpy.types.SpaceView3D.draw_handler_add(
                draw_distances, args, 'WINDOW', 'POST_PIXEL')
            self.report({'INFO'}, "Distance drawing enabled")
        else:
            bpy.types.SpaceView3D.draw_handler_remove(
                VIEW3D_OT_toggle_distance_draw._handle, 'WINDOW')
            VIEW3D_OT_toggle_distance_draw._handle = None
            self.report({'INFO'}, "Distance drawing disabled")
        context.area.tag_redraw()
        return {'FINISHED'}


def register():
    bpy.utils.register_class(VIEW3D_OT_toggle_distance_draw)


def unregister():
    bpy.utils.unregister_class(VIEW3D_OT_toggle_distance_draw)

