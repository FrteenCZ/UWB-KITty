import json


def distances(obj, targets):
    points = []
    if not obj or len(targets) == 0:
        return points

    for target in targets:
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

    data = json.dumps({"distances": points})
    print(f"Sent points: {len(points)} targets")

    return data.encode('utf-8')
