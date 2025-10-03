/**
 * Users Microservice Router
 * 
 * Handles all routes related to users management
 */

import { Router, Request, Response } from 'express';
import { BaseRouter } from './BaseRouter';
import { RouteConfig, CallMicroserviceFunction, HandleServiceErrorFunction } from '../types';

export class UsersRouter extends BaseRouter {
    public routeConfig: RouteConfig = {
        serviceName: 'users',
        basePath: '/users',
        endpoints: [
            {
                path: '',
                method: 'GET',
                handler: 'getAllUsers'
            },
            {
                path: '/:id',
                method: 'GET',
                handler: 'getUserById',
                validation: (req: Request) => this.validateNumericId(req.params.id, 'User ID')
            }
        ]
    };

    public setupRoutes(
        callMicroservice: CallMicroserviceFunction,
        handleServiceError: HandleServiceErrorFunction
    ): Router {
        // GET /users - Get all users
        this.router.get('/', async (req: Request, res: Response): Promise<void> => {
            const handler = this.createRouteHandler('GET', '/users');
            await handler(req, res, callMicroservice, handleServiceError);
        });

        // GET /users/:id - Get user by ID
        this.router.get('/:id', async (req: Request, res: Response): Promise<void> => {
            const handler = this.createRouteHandler(
                'GET', 
                '/users/:id',
                (req: Request) => this.validateNumericId(req.params.id, 'User ID')
            );
            await handler(req, res, callMicroservice, handleServiceError);
        });

        return this.router;
    }
}