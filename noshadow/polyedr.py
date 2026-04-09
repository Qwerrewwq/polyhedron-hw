import numpy as np
from math import pi
from common.r3 import R3
from common.tk_drawer import TkDrawer, x, y


class Edge:
    """ Ребро полиэдра """
    # Параметры конструктора: начало и конец ребра (точки в R3)

    def __init__(self, beg, fin):
        self.beg, self.fin = beg, fin


class Facet:
    """ Грань полиэдра """
    # Параметры конструктора: список вершин

    def __init__(self, vertexes):
        self.vertexes = vertexes


class Polyedr:
    """ Полиэдр """
    # Параметры конструктора: файл, задающий полиэдр

    def __init__(self, file):

        # списки вершин, рёбер и граней полиэдра
        self.vertexes, self.edges, self.facets = [], [], []
        
        # Numpy массивы для оптимизированных вычислений
        self.vertexes_array = None

        # список строк файла
        with open(file) as f:
            lines = f.readlines()
            
        # Первая строка: коэффициент гомотетии и углы Эйлера
        buf = lines[0].split()
        c = float(buf.pop(0))
        alpha, beta, gamma = (float(x_val) * pi / 180.0 for x_val in buf)
        
        # Вторая строка: число вершин, граней и рёбер
        nv, nf, ne = (int(x_val) for x_val in lines[1].split())
        
        # Чтение всех вершин
        raw_vertices = []
        for i in range(2, nv + 2):
            x_val, y_val, z_val = (float(x_val) for x_val in lines[i].split())
            raw_vertices.append((x_val, y_val, z_val))
        
        # Массовое применение поворота и масштабирования с помощью numpy
        rotated = R3.rotate_vertices(raw_vertices, alpha, beta, gamma)
        self.vertexes_array = rotated * c
        
        # Создание объектов R3 для обратной совместимости
        for i in range(nv):
            self.vertexes.append(R3(*self.vertexes_array[i]))
        
        # Чтение граней и создание рёбер
        line_idx = nv + 2
        for _ in range(nf):
            buf = lines[line_idx].split()
            line_idx += 1
            size = int(buf.pop(0))
            vertex_indices = [int(n) - 1 for n in buf]
            vertexes = [self.vertexes[n] for n in vertex_indices]
            
            # Задание рёбер грани
            for n in range(size):
                self.edges.append(Edge(vertexes[n - 1], vertexes[n]))
            
            # Задание самой грани
            self.facets.append(Facet(vertexes))

    # Метод изображения полиэдра
    def draw(self, tk):
        tk.clean()
        
        # Подготовка координат для массовой отрисовки
        lines_coords = []
        for e in self.edges:
            coord1 = (x(e.beg), y(e.beg))
            coord2 = (x(e.fin), y(e.fin))
            lines_coords.append((coord1, coord2))
        
        # Массовая отрисовка всех линий
        tk.draw_lines_batch(lines_coords)
