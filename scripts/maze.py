
from typing import *
from Box2D import b2
from collections import deque
from dataclasses import dataclass
from random import randrange

@dataclass
class Tile:
    up: bool
    down: bool
    left: bool
    right: bool

@dataclass
class Line:
    x0: float
    y0: float
    x1: float
    y1: float

@dataclass
class Rect:
    x: float
    y: float
    width: float
    height: float

@dataclass
class Coordinate:
    x: int
    y: int

Matrix = List[List[Tile]]
Path = List[Coordinate]

class Maze(object):
    __world: b2.world = None
    __ground: b2.body = None
    __body: b2.body = None

    __rows: int = 0
    __columns: int = 0
    __x: float = 0.0
    __y: float = 0.0
    __width: float = 0.0
    __height: float = 0.0
    __thickness: float = 0.0
    __tileWidth: float = 0.0
    __tileHeight: float = 0.0
    __matrix: Matrix = []
   
    def __init__(self, world: b2.world, ground: b2.body, rows: int, columns: int, x: float, y: float, height: float, width: float, thickness: float):
        self.__world = world
        self.__ground = ground
        self.__rows = rows
        self.__columns = columns
        self.__x = x
        self.__y = y
        self.__height = height
        self.__width = width
        self.__thickness = thickness
        self.__tileHeight = self.__height / self.__rows
        self.__tileWidth = self.__width / self.__columns
        self.__matrix = Maze.make(self.__rows, self.__columns)
        self.__createBody()
    
    @staticmethod
    def make(rows: int, columns: int) -> Matrix:
        matrix = [[Tile(True,True,True,True) for x in range(columns)] for y in range(rows)]
        tracking = deque()
        j, i = 0, 0
        tracking.append((j, i))
        while True:
            directions = []

            def unvisited(dj, di):
                return (matrix[j + dj][i + di].up
                    and matrix[j + dj][i + di].down
                    and matrix[j + dj][i + di].left
                    and matrix[j + dj][i + di].right)

            if (j > 0 and unvisited(-1, 0)):
                directions.append((-1, 0))
            if (j < rows - 1 and unvisited(+1, 0)):
                directions.append((+1, 0))
            if (i > 0 and unvisited(0, -1)):
                directions.append((0, -1))
            if (i < columns - 1 and unvisited(0, +1)):
                directions.append((0, +1))

            if (len(directions) > 0):
                choosen = randrange(len(directions))
                dj, di = directions[choosen]

                if (dj == +1):
                    matrix[j][i].down = False
                    matrix[j + 1][i].up = False
                elif (dj == -1):
                    matrix[j][i].up = False
                    matrix[j - 1][i].down = False
                elif (di == +1):
                    matrix[j][i].right = False
                    matrix[j][i + 1].left = False
                elif (di == -1):
                    matrix[j][i].left = False
                    matrix[j][i - 1].right = False

                j += dj
                i += di

                tracking.append((j, i))

            else:
                tracking.pop()
                
                if (len(tracking) == 0):
                    break

                j, i = tracking[-1]
        
        #matrix[0][0].up = False
        #matrix[-1][-1].down = False
        
        return matrix

    @staticmethod
    def print(matrix: Matrix, path: Path = []) -> None:
        SQUARE = '\xDB'
        FREE = ' '
        OCCUPIED = 'X' 

        print(SQUARE, end='')

        y = 0

        for x in range(len(matrix[y])):
            if (matrix[y][x].up):
                print(SQUARE, end='')
            else:
                print(FREE, end='')

            print(SQUARE, end='')
            
        print()

        for y in range(len(matrix)):
            if (matrix[y][0].left):
                print(SQUARE, end='')
            else:
                print(FREE, end='')

            for x in range(len(matrix[y])):
                found = False
                for coordinate in path:
                    if (coordinate.y == y and coordinate.x == x):
                        found = True
                        break
                if (found):
                    print(OCCUPIED, end='')
                else:
                    print(FREE, end='')

                if (matrix[y][x].down and matrix[y][x].right):
                    print(SQUARE, end='')
                elif (matrix[y][x].down):
                    print(FREE, end='')
                elif (matrix[y][x].right):
                    print(SQUARE, end='')
                else:
                    print(FREE, end='')

            print()

            print(SQUARE, end='')

            for x in range(len(matrix[y])):
                if (matrix[y][x].down and matrix[y][x].right):
                    print(SQUARE, end='')
                elif (matrix[y][x].down):
                    print(SQUARE, end='')
                elif (matrix[y][x].right):
                    print(FREE, end='')
                else:
                    print(FREE, end='')

                print(SQUARE, end='')

            print()

        return

    @staticmethod
    def lines(matrix: Matrix, height: float, width: float) -> List[Line]:
        lines = []

        tileHeight = height / len(matrix)
        tileWidth = width / len(matrix[0])

        def makeVertical(y, currentHeight, direction):
            start = 0.0
            end = 0.0
            for x in range(len(matrix[0])):
                if((direction == 'up' and matrix[y][x].up) or (direction == 'down' and matrix[y][x].down)):
                    end += tileWidth
                else:
                    if (end - start > 0.001):
                        lines.append(Line( start, currentHeight, end,  currentHeight ))
                    start = end + tileWidth
                    end = start

            if (end - start > 0.001):
                lines.append(Line(start, currentHeight, end, currentHeight))

        makeVertical(0, 0.0, 'up')

        for y in range(len(matrix)):
            makeVertical(y, (y + 1.0) * tileHeight, 'down')

        def makeHorizontal(x, currentWidth, direction):
            start = 0.0
            end = 0.0
            for y in range(len(matrix)):
                if((direction == 'left' and matrix[y][x].left) or (direction == 'right' and matrix[y][x].right)):
                    end += tileHeight
                else:
                    if (end - start > 0.001):
                        lines.append(Line( currentWidth, start, currentWidth, end ))
                    start = end + tileHeight
                    end = start

            if (end - start > 0.001):
                lines.append(Line( currentWidth, start, currentWidth, end ))

        makeHorizontal(0, 0.0, 'left')

        for x in range(len(matrix[0])):
            makeHorizontal(x, (x + 1.0) * tileWidth, 'right')

        return lines

    @staticmethod
    def rectangles(matrix: Matrix, x: float, y: float, height: float, width: float, thickness: float) -> List[Rect]:
        rectangles = []
        lines = Maze.lines(matrix,height,width)

        for line in lines:
            rectangles.append(Rect( x + line.x0, y + line.y0, line.x1 - line.x0 + thickness, line.y1 - line.y0 + thickness ))

        return rectangles
    
    def __createBody(self) -> None:
        if (self.__body != None):
            self.__world.DestroyBody(self.__body)

        self.__body = self.__world.CreateBody(
            type = b2.staticBody,
            position = (self.__x, self.__y),
            userData = 'maze'
        )

        rectangles = Maze.rectangles(self.__matrix, 0, 0, self.__width, self.__height, self.__thickness)
        for rect in rectangles:
            self.__body.CreateFixture(
                shape = b2.polygonShape(
                    vertices=[
                        (rect.x, rect.y),
                        (rect.x + rect.width, rect.y),
                        (rect.x + rect.width, rect.y + rect.height),
                        (rect.x, rect.y + rect.height)
                    ]
                ),
                density = 0.0,
                restitution = 0.4,
                categoryBits = 0x0001,
                maskBits = 0x0003,
                userData = (
                    'wall',
                    (172, 0, 172, 255),
                    (172, 0, 172, 127)
                )
            )

        self.__body.CreateFixture(
            shape=b2.circleShape(
                pos = (self.start() - self.__body.position),
                radius = min(self.__tileHeight, self.__tileWidth) / 4.0
            ),
            isSensor = True,
            userData = (
                'start',
                (0, 255, 0, 255),
                (0, 255, 0, 64)
            )
        )

        self.__body.CreateFixture(
            shape=b2.circleShape(
                pos = (self.end() - self.__body.position),
                radius = min(self.__tileHeight,self.__tileWidth) / 4.0
            ),
            isSensor = True,
            userData = (
                'end',
                (255, 0, 0, 255),
                (255, 0, 0, 64)
            )
        )

        return

    def start(self) -> b2.vec2:
        return self.toRealPoint(Coordinate(self.__columns - 1, self.__rows - 1))

    def end(self) -> b2.vec2:
        return self.toRealPoint(Coordinate( 0,0 ))

    def toLocalCoordinate(self, point: b2.vec2) -> Coordinate:
        y = ( point.y - self.__y ) / self.__tileHeight
        x = ( point.x - self.__x ) / self.__tileWidth
        return Coordinate( x, y )

    def toRealPoint(self, coordinate: Coordinate) -> b2.vec2:
        y = coordinate.y * self.__tileHeight + self.__tileHeight / 2.0 + self.__y
        x = coordinate.x * self.__tileWidth + self.__tileWidth / 2.0 + self.__x
        return b2.vec2(x, y)