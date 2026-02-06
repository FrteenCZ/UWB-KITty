import bpy # type: ignore
from ..globals import device_manager
from .serial_modal import SERIAL_OT_StartESP


_timer_handle = None


def tick_update():
    """Non-blocking update callback"""
    device_manager.update()
    return 0.016  # Continue every 16ms (~60 FPS)


class WM_OT_tick_start(bpy.types.Operator):
    """Start the tick update loop and load devices"""
    bl_idname = "wm.tick_start"
    bl_label = "Start Simulation"

    def execute(self, context):
        global _timer_handle
        
        if _timer_handle is not None:
            self.report({'WARNING'}, "Tick loop already running")
            return {'CANCELLED'}
        
        # Load devices from persistent properties
        serial_port = SERIAL_OT_StartESP._thread
        device_manager.load_devices_from_properties(context, serial_port)
        
        _timer_handle = bpy.app.timers.register(tick_update)
        self.report({'INFO'}, "Tick loop started")
        return {'FINISHED'}


class WM_OT_tick_stop(bpy.types.Operator):
    """Stop the tick update loop"""
    bl_idname = "wm.tick_stop"
    bl_label = "Stop Simulation"

    def execute(self, context):
        global _timer_handle
        
        if _timer_handle is None:
            self.report({'WARNING'}, "Tick loop not running")
            return {'CANCELLED'}
        
        bpy.app.timers.unregister(_timer_handle)
        _timer_handle = None
        
        # Clear the devices from the manager
        device_manager.clear_devices()
        
        self.report({'INFO'}, "Tick loop stopped")
        return {'FINISHED'}


def register():
    bpy.utils.register_class(WM_OT_tick_start)
    bpy.utils.register_class(WM_OT_tick_stop)


def unregister():
    global _timer_handle
    
    if _timer_handle is not None:
        bpy.app.timers.unregister(_timer_handle)
        _timer_handle = None
    
    bpy.utils.unregister_class(WM_OT_tick_start)
    bpy.utils.unregister_class(WM_OT_tick_stop)