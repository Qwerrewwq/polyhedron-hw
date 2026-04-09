from tkinter import *
import numpy as np

# Размер окна
SIZE = 900
# Коэффициент гомотетии
SCALE = 1.5


def x(p):
    """ преобразование x-координаты """
    return SIZE / 2 + SCALE * p.x


def y(p):
    """" преобразование y-координаты """
    return SIZE / 2 - SCALE * p.y


class TkDrawer:
    """ Графический интерфейс с оптимизацией для быстрой отрисовки """

    # Конструктор
    def __init__(self):
        self.root = Tk()
        self.root.title("Изображение проекции полиэдра")
        self.root.geometry(f"{SIZE+5}x{SIZE+5}")
        self.root.resizable(False, False)
        self.root.bind('<Control-c>', quit)
        self.canvas = Canvas(self.root, width=SIZE, height=SIZE)
        self.canvas.pack(padx=5, pady=5)
        # Кэш для координат
        self._coord_cache = {}

    # Завершение работы
    def close(self):
        self.root.quit()

    # Стирание существующей картинки
    def clean(self):
        self.canvas.create_rectangle(0, 0, SIZE, SIZE, fill="white")
        self.root.update()

    # Рисование линии
    def draw_line(self, p, q):
        self.canvas.create_line(x(p), y(p), x(q), y(q), fill="black", width=1)
        self.root.update()

    # Массовая отрисовка линий (оптимизировано)
    def draw_lines_batch(self, lines_coords):
        """
        lines_coords: список кортежей ((x1,y1), (x2,y2))
        Отрисовывает все линии за один проход без обновления после каждой
        """
        for coord in lines_coords:
            (x1, y1), (x2, y2) = coord
            self.canvas.create_line(x1, y1, x2, y2, fill="black", width=1)
        self.root.update()


if __name__ == "__main__":

    import time
    from r3 import R3
    tk = TkDrawer()
    tk.clean()
    tk.draw_line(R3(0.0, 0.0, 0.0), R3(100.0, 100.0, 0.0))
    tk.draw_line(R3(0.0, 0.0, 0.0), R3(0.0, 100.0, 0.0))
    time.sleep(5)
