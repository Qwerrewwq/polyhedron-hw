import numpy as np
from math import pi, cos, sin
from functools import reduce
from operator import add
from concurrent.futures import ThreadPoolExecutor
import multiprocessing as mp
from common.r3 import R3


class Segment:
    """ Одномерный отрезок """
    # Параметры конструктора: начало и конец отрезка (числа)

    def __init__(self, beg, fin):
        self.beg, self.fin = beg, fin

    # Отрезок вырожден?
    def is_degenerate(self):
        return self.beg >= self.fin

    # Пересечение с отрезком
    def intersect(self, other):
        if other.beg > self.beg:
            self.beg = other.beg
        if other.fin < self.fin:
            self.fin = other.fin
        return self

    # Разность отрезков
    # Разность двух отрезков всегда является списком из двух отрезков!
    def subtraction(self, other):
        return [Segment(
            self.beg, self.fin if self.fin < other.beg else other.beg),
            Segment(self.beg if self.beg > other.fin else other.fin, self.fin)]


class Edge:
    """ Ребро полиэдра """
    # Начало и конец стандартного одномерного отрезка
    SBEG, SFIN = 0.0, 1.0

    # Параметры конструктора: начало и конец ребра (точки в R3)
    def __init__(self, beg, fin):
        self.beg, self.fin = beg, fin
        # Список «просветов»
        self.gaps = [Segment(Edge.SBEG, Edge.SFIN)]

    # Учёт тени от одной грани
    def shadow(self, facet):
        # «Вертикальная» грань не затеняет ничего
        if facet.is_vertical():
            return
        # Нахождение одномерной тени на ребре
        shade = Segment(Edge.SBEG, Edge.SFIN)
        for u, v in zip(facet.vertexes, facet.v_normals()):
            shade.intersect(self.intersect_edge_with_normal(u, v))
            if shade.is_degenerate():
                return

        shade.intersect(
            self.intersect_edge_with_normal(
                facet.vertexes[0], facet.h_normal()))
        if shade.is_degenerate():
            return
        # Преобразование списка «просветов», если тень невырождена
        gaps = [s.subtraction(shade) for s in self.gaps]
        self.gaps = [
            s for s in reduce(add, gaps, []) if not s.is_degenerate()]

    # Преобразование одномерных координат в трёхмерные
    def r3(self, t):
        return self.beg * (Edge.SFIN - t) + self.fin * t

    # Пересечение ребра с полупространством, задаваемым точкой (a)
    # на плоскости и вектором внешней нормали (n) к ней
    def intersect_edge_with_normal(self, a, n):
        f0, f1 = n.dot(self.beg - a), n.dot(self.fin - a)
        if f0 >= 0.0 and f1 >= 0.0:
            return Segment(Edge.SFIN, Edge.SBEG)
        if f0 < 0.0 and f1 < 0.0:
            return Segment(Edge.SBEG, Edge.SFIN)
        x = - f0 / (f1 - f0)
        return Segment(Edge.SBEG, x) if f0 < 0.0 else Segment(x, Edge.SFIN)


class Facet:
    """ Грань полиэдра """
    # Параметры конструктора: список вершин

    def __init__(self, vertexes):
        self.vertexes = vertexes
        self._h_norm = None
        self._v_norms = None
        self._is_vert = None
        self._center = None

    # «Вертикальна» ли грань?
    def is_vertical(self):
        return self.h_normal().dot(Polyedr.V) == 0.0

    # Нормаль к «горизонтальному» полупространству
    def h_normal(self):
        if self._h_norm is None:
            n = (
                self.vertexes[1] - self.vertexes[0]).cross(
                self.vertexes[2] - self.vertexes[0])
            self._h_norm = n * (-1.0) if n.dot(Polyedr.V) < 0.0 else n
        return self._h_norm

    # Нормали к «вертикальным» полупространствам, причём k-я из них
    # является нормалью к грани, которая содержит ребро, соединяющее
    # вершины с индексами k-1 и k
    def v_normals(self):
        if self._v_norms is None:
            self._v_norms = [self._vert(x) for x in range(len(self.vertexes))]
        return self._v_norms

    # Вспомогательный метод
    def _vert(self, k):
        n = (self.vertexes[k] - self.vertexes[k - 1]).cross(Polyedr.V)
        return n * \
            (-1.0) if n.dot(self.vertexes[k - 1] - self.center()) < 0.0 else n

    # Центр грани
    def center(self):
        if self._center is None:
            self._center = sum(self.vertexes, R3(0.0, 0.0, 0.0)) * \
                (1.0 / len(self.vertexes))
        return self._center

    def get_data_for_processing(self):
        """Подготовка данных для передачи в процесс"""
        return {
            'vertexes': [(v.x, v.y, v.z) for v in self.vertexes],
            'h_normal': (self.h_normal().x, self.h_normal().y, self.h_normal().z),
            'v_normals': [(n.x, n.y, n.z) for n in self.v_normals()],
            'is_vertical': self.is_vertical()
        }


def _process_edge_task_optimized(args):
    """Оптимизированная функция для обработки ребра с использованием NumPy"""
    edge_beg, edge_fin, facets_data = args
    
    SBEG, SFIN = 0.0, 1.0
    
    # Восстанавливаем векторы из кортежей
    beg = np.array(edge_beg, dtype=np.float64)
    fin = np.array(edge_fin, dtype=np.float64)
    edge_dir = fin - beg
    
    # Инициализируем просветы как список интервалов [beg, fin]
    gaps = [[SBEG, SFIN]]
    
    for facet in facets_data:
        if facet['is_vertical']:
            continue
        
        h_norm = np.array(facet['h_normal'], dtype=np.float64)
        v_normals = np.array(facet['v_normals'], dtype=np.float64)
        vertexes = np.array(facet['vertexes'], dtype=np.float64)
        
        # Находим тень от этой грани
        shade_beg, shade_fin = SBEG, SFIN
        
        # Векторизованная проверка пересечения с вертикальными полупространствами
        # Вычисляем f0 и f1 для всех нормалей сразу
        diff_beg = beg - vertexes
        diff_fin = fin - vertexes
        
        f0_arr = np.sum(v_normals * diff_beg, axis=1)
        f1_arr = np.sum(v_normals * diff_fin, axis=1)
        
        # Проверяем условия
        all_positive = np.all((f0_arr >= 0.0) & (f1_arr >= 0.0))
        if all_positive:
            break  # Ребро полностью в тени
        
        # Обрабатываем каждое вертикальное полупространство
        for i in range(len(vertexes)):
            f0, f1 = f0_arr[i], f1_arr[i]
            
            if f0 >= 0.0 and f1 >= 0.0:
                shade_beg, shade_fin = SFIN, SBEG
                break
            elif f0 < 0.0 and f1 < 0.0:
                continue
            else:
                denom = f1 - f0
                if abs(denom) > 1e-10:
                    x = -f0 / denom
                    if f0 < 0.0:
                        shade_fin = min(shade_fin, x)
                    else:
                        shade_beg = max(shade_beg, x)
            
            if shade_beg >= shade_fin:
                break
        
        if shade_beg >= shade_fin:
            continue
        
        # Проверяем пересечение с горизонтальным полупространством
        u0 = vertexes[0]
        f0 = np.dot(h_norm, beg - u0)
        f1 = np.dot(h_norm, fin - u0)
        
        if f0 >= 0.0 and f1 >= 0.0:
            continue  # Полностью в тени
        elif f0 < 0.0 and f1 < 0.0:
            pass  # Полностью вне тени
        else:
            denom = f1 - f0
            if abs(denom) > 1e-10:
                x = -f0 / denom
                if f0 < 0.0:
                    shade_fin = min(shade_fin, x)
                else:
                    shade_beg = max(shade_beg, x)
        
        if shade_beg >= shade_fin:
            continue
        
        # Вычитаем тень из просветов
        new_gaps = []
        for gap in gaps:
            g_beg, g_fin = gap
            # Часть до тени
            if g_beg < shade_beg:
                new_gaps.append([g_beg, min(g_fin, shade_beg)])
            # Часть после тени
            if g_fin > shade_fin:
                new_gaps.append([max(g_beg, shade_fin), g_fin])
        
        gaps = [g for g in new_gaps if g[0] < g[1]]
        
        if not gaps:
            break
    
    return (edge_beg, edge_fin, gaps)


class Polyedr:
    """ Полиэдр """
    # вектор проектирования
    V = R3(0.0, 0.0, 1.0)

    # Параметры конструктора: файл, задающий полиэдр
    def __init__(self, file):

        # списки вершин, рёбер и граней полиэдра
        self.vertexes, self.edges, self.facets = [], [], []

        # список строк файла
        with open(file) as f:
            lines = f.readlines()

        # обрабатываем первую строку; buf - вспомогательный массив
        buf = lines[0].split()
        # коэффициент гомотетии
        c = float(buf.pop(0))
        # углы Эйлера, определяющие вращение
        alpha, beta, gamma = (float(x) * pi / 180.0 for x in buf)

        # во второй строке число вершин, граней и рёбер полиэдра
        nv, nf, ne = (int(x) for x in lines[1].split())

        # предзагрузка всех вершин для быстрого доступа
        raw_vertexes = []
        for i in range(2, nv + 2):
            x, y, z = (float(x) for x in lines[i].split())
            raw_vertexes.append((x, y, z))

        # Векторизованное применение трансформаций ко всем вершинам
        rot_z_alpha = np.array([
            [cos(alpha), -sin(alpha), 0],
            [sin(alpha), cos(alpha), 0],
            [0, 0, 1]], dtype=np.float64)
        rot_y_beta = np.array([
            [cos(beta), 0, sin(beta)],
            [0, 1, 0],
            [-sin(beta), 0, cos(beta)]], dtype=np.float64)
        rot_z_gamma = np.array([
            [cos(gamma), -sin(gamma), 0],
            [sin(gamma), cos(gamma), 0],
            [0, 0, 1]], dtype=np.float64)
        combined_rot = rot_z_gamma @ rot_y_beta @ rot_z_alpha

        # Применяем трансформации ко всем вершинам сразу через numpy
        raw_arr = np.array(raw_vertexes, dtype=np.float64)
        transformed = (raw_arr @ combined_rot.T) * c

        # Создаём объекты R3 из трансформированных данных
        for i in range(nv):
            self.vertexes.append(R3(transformed[i, 0], transformed[i, 1], transformed[i, 2]))

        # Обработка граней и рёбер
        line_idx = nv + 2
        raw_edges = []  # Храним рёбра как кортежи координат
        while line_idx < len(lines):
            buf = lines[line_idx].split()
            line_idx += 1
            # количество вершин очередной грани
            size = int(buf.pop(0))
            # массив вершин этой грани
            vertexes = [self.vertexes[int(n) - 1] for n in buf]
            # задание рёбер грани
            for n in range(size):
                beg, fin = vertexes[n - 1], vertexes[n]
                raw_edges.append(((beg.x, beg.y, beg.z), (fin.x, fin.y, fin.z)))
                # Сохраняем также объекты Edge для совместимости с тестами
                self.edges.append(Edge(beg, fin))
            # задание самой грани
            self.facets.append(Facet(vertexes))
        
        # Сохраняем рёбра как список кортежей
        self._raw_edges = raw_edges

    # Метод изображения полиэдра с многопоточной обработкой теней
    def draw(self, tk):
        tk.clean()
        
        # Подготавливаем данные о гранях для передачи в потоки
        facets_data = [f.get_data_for_processing() for f in self.facets]
        
        # Формируем задачи для каждого ребра
        tasks = [(beg, fin, facets_data) for beg, fin in self._raw_edges]
        
        # Используем ThreadPoolExecutor для параллельной обработки на всех ядрах
        # Потоки эффективнее процессов для задач с большим количеством данных
        num_workers = mp.cpu_count()
        with ThreadPoolExecutor(max_workers=num_workers) as executor:
            results = list(executor.map(_process_edge_task_optimized, tasks))
        
        # Рисуем все рёбра по результатам обработки
        for edge_beg, edge_fin, gaps in results:
            beg_vec = R3(*edge_beg)
            fin_vec = R3(*edge_fin)
            for gap in gaps:
                t_beg, t_fin = gap
                p1 = beg_vec * (1.0 - t_beg) + fin_vec * t_beg
                p2 = beg_vec * (1.0 - t_fin) + fin_vec * t_fin
                tk.draw_line(p1, p2)
        
        # Обновляем экран один раз после отрисовки всех линий
        tk.flush()
