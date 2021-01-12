#!/usr/bin/env python
# -*- coding: utf-8 -*-

from math import *
from typing import *
from enum import Enum
from Box2D import b2

class Move(Enum):
    STOP = 0
    MOVE_FORWARD = 1
    MOVE_BACKWARD = 2
    ROTATE_LEFT = 3
    ROTATE_RIGHT = 4

class Color(Enum):
    BLACK = 0
    RED = 1
    GREEN = 2
    BLUE = 3
    YELLOW = 4
    MAGENTA = 5
    CYAN = 6
    WHITE = 7

class SensorCallback(b2.rayCastCallback):
    filter: b2.filter = None
    valid: bool = False
    point: b2.vec2 = b2.vec2(0,0)
    
    def __init__(self, filter: b2.filter):
        super(SensorCallback, self).__init__()
        self.filter = filter
        self.valid = False
        self.point = b2.vec2(0,0)
        return

    def ReportFixture(self, fixture: b2.fixture, point: b2.vec2, normal: b2.vec2, fraction: float):
        filter = fixture.filterData
        if ((self.filter.maskBits & filter.categoryBits) != 0 and (filter.maskBits & self.filter.categoryBits) != 0):
            self.valid = True
            self.point = b2.vec2(point)
            return fraction
        return -1

class Car(object):
    __world: b2.world = None
    __ground: b2.body = None
    __body: b2.body = None
    __move = Move.STOP
    __collision = False
    __sensors = {
        0: 0.0
    }
    __sensorsBodies = {
        0: None
    }

    def __init__(self, world: b2.world, ground: b2.body, position: b2.vec2):
        self.__world = world
        self.__ground = ground
        self.__createBody(position)
        return

    def __createBody(self, position: b2.vec2) -> None:
        if (self.__body != None):
            self.__world.DestroyBody(self.__body)

        self.__body = self.__world.CreateBody(
            type = b2.dynamicBody,
            position = position,
            angle = b2.pi,
            angularDamping = 6.0,
            userData = 'car'
        )

        WIDTH = 0.2
        HEIGHT = 0.2

        self.__body.CreateFixture(
            shape = b2.polygonShape(
                vertices=[
                    (0, 0),
                    (WIDTH, 0),
                    (WIDTH, HEIGHT),
                    (0, HEIGHT)
                ]
            ),
            density = 2.0,
            restitution = 0.5,
            categoryBits = 0x0002,
            maskBits = 0x0001,
            userData = (
                'chassis',
                (64, 64, 255, 255),
                (64, 64, 255, 127)
            )
        )

        #self.__body.CreateFixture(
        #    shape = b2.polygonShape(
        #        vertices=[
        #            (WIDTH / 2, HEIGHT / 3),
        #            (2 * WIDTH / 3, 2 * HEIGHT / 3),
        #            (1 * WIDTH / 3, 2 * HEIGHT / 3)
        #        ]
        #    ),
        #    isSensor=True,
        #    categoryBits = 0x0002,
        #    maskBits = 0x0001,
        #    userData = (
        #        'direction',
        #        (0, 0, 255, 255),
        #        (0, 0, 255, 127)
        #    )
        #)

        GRAVITY = 10.0
        INERTIA = self.__body.inertia
        MASS = self.__body.mass
        RADIUS = sqrt(2.0 * INERTIA / MASS)

        self.__world.CreateFrictionJoint(
            bodyA=self.__ground,
            bodyB=self.__body,
            localAnchorA=(0, 0),
            localAnchorB=self.__body.localCenter,
            collideConnected=True,
            maxForce = 1.35 * MASS * GRAVITY,
            maxTorque = 8.975 * MASS * RADIUS * GRAVITY,
            userData='friction'
        )

        for angle in self.__sensorsBodies:
            self.__sensorsBodies[angle] = self.__world.CreateBody(
                type = b2.staticBody,
                position = (0,0),
                userData = 'sensor'
            )
            self.__sensorsBodies[angle].CreateFixture(
                shape = b2.edgeShape(
                    vertices=[
                        (-0.05, 0),
                        (+0.05, 0)
                    ]
                ),
                isSensor = True,
                userData = (
                    'sensor',
                    (0, 255, 0, 255),
                    (0, 255, 0, 64)
                )
            )
            self.__sensorsBodies[angle].CreateFixture(
                shape = b2.edgeShape(
                    vertices=[
                        (0, -0.05),
                        (0, +0.05)
                    ]
                ),
                isSensor = True,
                userData = (
                    'sensor',
                    (0, 255, 0, 255),
                    (0, 255, 0, 64)
                )
            )

        return

    def reset(self) -> None:
        self.__body.SetLinearVelocity(( 0, 0 ))
        self.__body.SetAngularVelocity(0)
        self.__body.SetTransform(( 0, 0 ), 0)

        self.__move = Move.STOP
        self.__collision = False
    
        return

    def step(self) -> None:
        self.__body.ApplyTorque(1.85, True)

        for angle in self.__sensors:
            radians = (angle / 180.0) * b2.pi
            
            maxDistance = 4.0
            start = self.__body.GetWorldPoint(b2.mul(b2.rot( radians ), b2.vec2( 0.0, 0.10 )))
            end = self.__body.GetWorldPoint(b2.mul(b2.rot( radians ), b2.vec2( 0.0, 0.10 + maxDistance )))

            filter = b2.filter(
                categoryBits = 0x0002,
                maskBits = 0x0001
            )
            callback = SensorCallback(filter)
            callback.point = start

            self.__world.RayCast(callback, start, end)
            if (callback.valid):
                self.__sensorsBodies[angle].position = callback.point
            else:
                distance = 9999.9
                self.__sensorsBodies[angle].position = (9999.9,9999.9)

        return

    