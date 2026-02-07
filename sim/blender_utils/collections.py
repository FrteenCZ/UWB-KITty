import bpy


def cleanup_collection(coll, keep_names=None):
    keep_names = set(keep_names or [])

    for obj in list(coll.objects):
        if obj.name not in keep_names:
            bpy.data.objects.remove(obj, do_unlink=True)
