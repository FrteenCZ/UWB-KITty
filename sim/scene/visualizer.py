import bpy  # type: ignore
import numpy as np
from ..blender_utils.objects import ensure_arrow, ensure_empty
from ..blender_utils.collections import cleanup_collection


class Visualizer:
    def __init__(self, device):
        self.device = device
        self.trilateration_collection = self._get_or_create_collection(
            "Trilateration")
        self.kalman_collection = self._get_or_create_collection("Kalman")

    def _get_or_create_collection(self, name):
        coll = bpy.data.collections.get(name)
        if not coll:
            coll = bpy.data.collections.new(name)
            bpy.context.scene.collection.children.link(coll)
        return coll

    def update(self):
        self._update_trilateration_visuals()
        self._update_kalman_visuals()

        # if self.device.obj and self.device.pos:
        #     self.device.obj.location = self.device.pos

    def _update_trilateration_visuals(self):
        dev = self.device
        if dev.pos is None or dev.alpha is None or dev.null_space is None:
            return

        [x, y, z] = dev.pos
        alpha = dev.alpha
        null_space = np.array(dev.null_space)
        coll = self.trilateration_collection

        if len(null_space) == 0:
            cleanup_collection(coll, ["Solution"])
            ensure_empty("Solution", "CUBE", scale=(
                1.2, 1.2, 1.2), location=(x, y, z), coll=coll)

        elif len(null_space) == 1:
            cleanup_collection(
                coll, ["Center", "Normal", "Solution", "AntiNormal", "AntiSolution"])
            ensure_empty("Center", "CUBE", scale=(0.1, 0.1, 0.1),
                         location=(x, y, z), coll=coll)
            ensure_arrow("Normal", (x, y, z),
                         null_space[0] * alpha, relative=True, coll=coll)
            ensure_empty("Solution", "CUBE", scale=(1.2, 1.2, 1.2),
                         location=(x, y, z) + alpha * null_space[0], coll=coll)
            ensure_arrow("AntiNormal", (x, y, z), -
                         null_space[0] * alpha, relative=True, coll=coll)
            ensure_empty("AntiSolution", "CUBE", scale=(1.2, 1.2, 1.2), location=(
                x, y, z) - alpha * null_space[0], coll=coll)

        elif len(null_space) == 2:
            cleanup_collection(coll, ["Center", "1DSolutionSpace"])
            ensure_empty("Center", "CUBE", scale=(0.1, 0.1, 0.1),
                         location=(x, y, z), coll=coll)
            normalVec = np.cross(null_space[0], null_space[1])
            r = np.sqrt(normalVec[0]**2 + normalVec[1]**2 + normalVec[2]**2)
            theta = np.arccos(normalVec[2] / r) if r != 0 else 0.0
            phi = np.arctan2(normalVec[1], normalVec[0])
            ensure_empty("1DSolutionSpace", "CIRCLE", scale=(alpha, alpha, alpha), location=(
                x, y, z), rotation=(np.pi/2 - theta, 0, phi - np.pi/2), coll=coll)

        elif len(null_space) == 3:
            cleanup_collection(coll, ["Center", "0DSolutionSpace"])
            ensure_empty("Center", "CUBE", scale=(0.1, 0.1, 0.1),
                         location=(x, y, z), coll=coll)
            obj = bpy.data.objects.get("0DSolutionSpace")
            if obj is None:
                bpy.ops.mesh.primitive_uv_sphere_add()
                obj = bpy.context.active_object
                obj.name = "0DSolutionSpace"
                coll.objects.link(obj)
                # Remove from default collection
                if "Collection" in bpy.data.collections:
                    bpy.data.collections["Collection"].objects.unlink(obj)
            obj.location = (x, y, z)
            obj.scale = (alpha, alpha, alpha)

    def _update_kalman_visuals(self):
        dev = self.device
        if dev.epos is None or dev.vel is None:
            return

        [x, y, z] = dev.epos
        [vx, vy, vz] = dev.vel
        coll = self.kalman_collection

        ensure_empty("Kalman", "PLAIN_AXES", location=(x, y, z), coll=coll)
        ensure_arrow("KalmanVelocity", (x, y, z),
                     (vx, vy, vz), relative=True, coll=coll)
