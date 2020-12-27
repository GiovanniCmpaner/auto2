#!/usr/bin/env python
# -*- coding: utf-8 -*-

from typing import *
import pygame
import Box2D
import time
from maze import Maze

from Box2D import b2

from Box2D import (b2World, b2AABB, b2CircleShape, b2Color, b2Vec2, b2BodyDef)
from Box2D import (b2ContactListener, b2DestructionListener, b2DrawExtended)
from Box2D import (b2Fixture, b2FixtureDef, b2Joint)
from Box2D import (b2GetPointStates, b2QueryCallback, b2Random)
from Box2D import (b2_addState, b2_dynamicBody, b2_epsilon, b2_persistState)

TARGET_FPS = 60
PPM = 50.0
TIMESTEP = 1.0 / TARGET_FPS
VEL_ITERS, POS_ITERS = 10, 10
SCREEN_WIDTH, SCREEN_HEIGHT = 800, 600
SCREEN_OFFSETX, SCREEN_OFFSETY = 50, 50
colors = {
    b2.staticBody: (255, 255, 255, 255),
    b2.dynamicBody: (127, 127, 127, 255),
    b2.kinematicBody: (127, 127, 230, 255),
}

pygame.init()
screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
clock = pygame.time.Clock()
world = b2World()

def createGround() -> b2.body:
    ground = world.CreateBody(
        type = b2.staticBody,
        position = (0,0),
        userData = "ground"
    )
    
    ground.CreateFixture(
        shape = b2.edgeShape(
            vertices = [(-10,0),(10,0)]
        ),
        density = 0,
        restitution = 0.4,
        categoryBits = 0x0000,
        maskBits = 0x0003,
        userData = "wall"
    )

    return ground
    
def fix_vertices(vertices):
    return [(int(SCREEN_OFFSETX + v[0]), int(SCREEN_OFFSETY + v[1])) for v in vertices]

# ---------------------------------------------------------------------------------------------------------
def _draw_polygon(polygon, screen, body, fixture):
    transform = body.transform
    vertices = fix_vertices([transform * v * PPM for v in polygon.vertices])
    pygame.draw.polygon(
        screen, [c / 2.0 for c in colors[body.type]], vertices, 0)
    pygame.draw.polygon(screen, colors[body.type], vertices, 1)
b2.polygonShape.draw = _draw_polygon

def _draw_circle(circle, screen, body, fixture):
    position = fix_vertices([body.transform * circle.pos * PPM])[0]
    pygame.draw.circle(screen, colors[body.type],
                       position, int(circle.radius * PPM))
b2.circleShape.draw = _draw_circle

def _draw_edge(edge, screen, body, fixture):
    vertices = fix_vertices(
        [body.transform * edge.vertex1 * PPM, body.transform * edge.vertex2 * PPM])
    pygame.draw.line(screen, colors[body.type], vertices[0], vertices[1])
b2.edgeShape.draw = _draw_edge

def _draw_loop(loop, screen, body, fixture):
    transform = body.transform
    vertices = fix_vertices([transform * v * PPM for v in loop.vertices])
    v1 = vertices[-1]
    for v2 in vertices:
        pygame.draw.line(screen, colors[body.type], v1, v2)
        v1 = v2
b2.loopShape.draw = _draw_loop
# ---------------------------------------------------------------------------------------------------------
ground = createGround()

matrix = Maze.make(5, 5)
Maze.print(matrix)
Maze.rectangles(matrix, 0, 0, 100, 100, 3)
maze = Maze(world,ground,5,5,5,5,5,5, 0.02)

running = True
while running:
    clock.tick(TARGET_FPS)
    screen.fill((0, 0, 0))
    for e in pygame.event.get():
        if e.type == pygame.QUIT:
            running = False
    
    # Step the world
    world.Step(TIMESTEP, VEL_ITERS, POS_ITERS)
    world.ClearForces()
    
    # Draw the world
    for body in world.bodies:
        for fixture in body.fixtures:
            fixture.shape.draw(screen, body, fixture)
    
    pygame.draw.rect(screen, (0, 255, 255), pygame.Rect(0, 0, 100, 30))
    
    pygame.display.flip()
    
pygame.quit()