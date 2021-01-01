#!/usr/bin/env python
# -*- coding: utf-8 -*-

from typing import *

import time
import pygame
import pygame.freetype
import pygame.gfxdraw

from maze import Maze

from Box2D import b2
from Box2D import (b2World, b2AABB, b2CircleShape, b2Color, b2Vec2, b2BodyDef)
from Box2D import (b2ContactListener, b2DestructionListener, b2DrawExtended)
from Box2D import (b2Fixture, b2FixtureDef, b2Joint)
from Box2D import (b2GetPointStates, b2QueryCallback, b2Random)
from Box2D import (b2_addState, b2_dynamicBody, b2_epsilon, b2_persistState)

TARGET_FPS = 60
PPM = 60.0
TIMESTEP = 1.0 / TARGET_FPS
VEL_ITERS, POS_ITERS = 5, 5
SCREEN_WIDTH, SCREEN_HEIGHT = 800, 600
SCREEN_OFFSETX, SCREEN_OFFSETY = 10, 10

pygame.init()
font = pygame.freetype.Font("C:/Windows/Fonts/Arial.ttf", 24)
screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
clock = pygame.time.Clock()
world = b2World()

def show_fps(screen, clock) -> None:
    font.render_to(screen, (0, 0), str(clock.get_fps()), (255,0,0))

def createGround() -> b2.body:
    ground = world.CreateBody(
        type = b2.staticBody,
        position = (0,0),
        userData = 'ground'
    )
    
    ground.CreateFixture(
        shape = b2.chainShape(
            vertices = [(0,0),(0,100),(100,100),(100,0)]
        ),
        density = 0,
        restitution = 0.4,
        categoryBits = 0x0000,
        maskBits = 0x0003,
        userData=(
            'ground',
            (0,0,0,0),
            (0,0,0,0)
        )
    )

    return ground
    
def fix_vertices(vertices):
    return [(int(SCREEN_OFFSETX + v[0]), int(SCREEN_OFFSETY + v[1])) for v in vertices]

# ---------------------------------------------------------------------------------------------------------
def _draw_polygon(polygon, screen, body, fixture, border, fill):
    transform = body.transform
    vertices = fix_vertices([transform * v * PPM for v in polygon.vertices])
    pygame.gfxdraw.filled_polygon(screen, vertices, fill)
    pygame.gfxdraw.polygon(screen, vertices, border)
b2.polygonShape.draw = _draw_polygon

def _draw_circle(circle, screen, body, fixture, border, fill):
    position = fix_vertices([body.transform * circle.pos * PPM])[0]
    pygame.gfxdraw.filled_circle(screen, position[0], position[1], int(circle.radius * PPM), fill)
    pygame.gfxdraw.circle(screen, position[0], position[1], int(circle.radius * PPM), border)
b2.circleShape.draw = _draw_circle

def _draw_edge(edge, screen, body, fixture, border, fill):
    vertices = fix_vertices([body.transform * edge.vertex1 * PPM, body.transform * edge.vertex2 * PPM])
    pygame.draw.line(screen, border, vertices[0], vertices[1])
b2.edgeShape.draw = _draw_edge

def _draw_loop(loop, screen, body, fixture, border, fill):
    transform = body.transform
    vertices = fix_vertices([transform * v * PPM for v in loop.vertices])
    v1 = vertices[-1]
    for v2 in vertices:
        pygame.draw.line(screen, border, v1, v2)
        v1 = v2
b2.loopShape.draw = _draw_loop
# ---------------------------------------------------------------------------------------------------------
ground = createGround()

mazes = []
for y in range(10):
    for x in range(10):
        mazes.append(Maze(world, ground, 5, 5, 0.5 + x * 3.2, 0.5 + y * 3.2, 3, 3, 0.05))

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
            (name,border,fill) = fixture.userData
            fixture.shape.draw(screen, body, fixture, border, fill)
    
    #show_fps(screen, clock)
    pygame.display.flip()
    
pygame.quit()