import bpy  # type: ignore
from ..utils.ESPcom import SerialThread


def ensure_arrow(name, start=(0, 0, 0), end=(0, 0, 0), relative=False):
    """Create or update an arrow (mesh line) between start and end."""
    if relative:
        end = (start[0] + end[0], start[1] + end[1], start[2] + end[2])

    obj = bpy.data.objects.get(name)
    if obj is None:
        # Create a mesh line
        mesh = bpy.data.meshes.new(name + "Mesh")
        obj = bpy.data.objects.new(name, mesh)
        bpy.context.collection.objects.link(obj)

    # Update geometry
    mesh = obj.data
    mesh.clear_geometry()
    mesh.from_pydata([start, end], [(0, 1)], [])
    mesh.update()
    return obj


def update_scene(thread):
    # Trilateration State
    if thread.trilateration_cords is not None and thread.alpha is not None and thread.null_space is not None:
        [x, y, z] = thread.trilateration_cords[0]
        alpha = thread.alpha

        for i, vec in enumerate(thread.null_space):
            ensure_arrow(f"NullSpace{i+1}", (x, y, z),
                         vec, relative=True)  # * alpha

        obj = bpy.data.objects.get("Trilateration")
        if obj is None:
            bpy.ops.object.empty_add(type='CUBE')
            obj = bpy.context.active_object
            obj.name = "Trilateration"
            obj.scale = (1.2, 1.2, 1.2)

        obj.location = (x, y, z)

    # Kalman Filter State
    if thread.kalman_cords is not None:
        [x, y, z, vx, vy, vz] = thread.kalman_cords[0]

        obj = bpy.data.objects.get("Kalman")
        if obj is None:
            bpy.ops.object.empty_add(type='PLAIN_AXES')
            obj = bpy.context.active_object
            obj.name = "Kalman"

        obj.location = (x, y, z)

        ensure_arrow("KalmanVelocity", (x, y, z),
                     (vx, vy, vz), relative=True)


def collect_points():
    points = []
    obj = bpy.data.objects.get("Cube")  # your tag
    if not obj:
        return points

    collection = bpy.data.collections.get("DistanceTargets")
    if not collection:
        return points

    for target in collection.objects:
        if target == obj:
            continue

        points.append({
            "name": target.name,
            "location": {
                "x": target.location.x,
                "y": target.location.y,
                "z": target.location.z,
            },
            "distance": (target.location - obj.location).length
        })
    return points


class SERIAL_OT_ModalESP(bpy.types.Operator):
    """Continuously read from ESP"""
    bl_idname = "wm.serial_modal_esp"
    bl_label = "ESP Serial Modal Operator"

    _timer = None
    _thread = None

    def modal(self, context, event):
        if event.type == 'TIMER':
            points = collect_points()
            self._thread.send_trilateration(points)

            update_scene(self._thread)

        if not self._thread.keep_running:
            self.cancel(context)
            return {'CANCELLED'}

        return {'PASS_THROUGH'}

    def execute(self, context):
        self._thread = SerialThread(port='/dev/ttyUSB0', baudrate=115200)
        self._thread.start()

        wm = context.window_manager
        self._timer = wm.event_timer_add(0.5, window=context.window)
        wm.modal_handler_add(self)
        return {'RUNNING_MODAL'}

    def cancel(self, context):
        if self._thread:
            self._thread.stop()
            self._thread.join()
        wm = context.window_manager
        wm.event_timer_remove(self._timer)
        print("Modal operator stopped.")


def menu_func(self, context):
    self.layout.operator(SERIAL_OT_ModalESP.bl_idname)


def register():
    bpy.utils.register_class(SERIAL_OT_ModalESP)
    bpy.types.TOPBAR_MT_file.append(menu_func)


def unregister():
    bpy.utils.unregister_class(SERIAL_OT_ModalESP)
    bpy.types.TOPBAR_MT_file.remove(menu_func)
