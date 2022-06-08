
with open('./Teapot.txt') as f:
    lines = f.readlines()
    vertex = list(map(float, lines[0].split(',')))
    normal = list(map(float, lines[1].split(',')))

    for i in range(0, len(vertex), 3):
        print(f"{vertex[i]} {vertex[i + 1]} {vertex[i + 2]} {normal[i]} {normal[i + 1]} {normal[i + 2]}")
