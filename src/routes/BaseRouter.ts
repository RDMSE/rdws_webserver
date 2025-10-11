/**
 * Base Router Class for Microservices
 *
 * Abstract base class that provides common functionality for all microservice routers
 */

import { Router, Request, Response } from 'express';
import {
  RouteConfig,
  CallMicroserviceFunction,
  HandleServiceErrorFunction,
  ErrorResponse,
  MicroserviceRouter
} from '../types';

export abstract class BaseRouter implements MicroserviceRouter {
  public abstract routeConfig: RouteConfig;
  protected router: Router;

  constructor() {
    this.router = Router();
  }

  /**
   * Validates numeric ID parameters
   */
  protected validateNumericId(id: string, fieldName: string = 'ID'): string | null {
    if (!/^\d+$/.test(id)) {
      return `${fieldName} must be a number`;
    }
    return null;
  }

  /**
   * Creates a standardized error response
   */
  protected createErrorResponse(message: string, requestId: string): ErrorResponse {
    return {
      error: true,
      message,
      requestId
    };
  }

  /**
   * Generic route handler that validates parameters and calls microservice
   */
  protected createRouteHandler(
    method: string,
    pathTemplate: string,
    validation?: (req: Request) => string | null
  ) {
    return async (
      req: Request,
      res: Response,
      callMicroservice: CallMicroserviceFunction,
      handleServiceError: HandleServiceErrorFunction
    ): Promise<void> => {
      try {
        // Apply validation if provided
        if (validation) {
          const validationError = validation(req);
          if (validationError) {
            const errorResponse = this.createErrorResponse(validationError, req.requestId);
            res.status(400).json(errorResponse);
            return;
          }
        }

        // Build the actual path by replacing parameters
        let actualPath = pathTemplate;
        Object.keys(req.params).forEach(param => {
          actualPath = actualPath.replace(`:${param}`, req.params[param]);
        });

        // Call microservice
        const result = await callMicroservice(
          this.routeConfig.serviceName,
          method,
          actualPath,
          req.requestId
        );

        res.json(result);
      } catch (error) {
        handleServiceError(error as Error, req, res);
      }
    };
  }

  /**
   * Sets up all routes defined in routeConfig
   * Must be implemented by each microservice router
   */
  public abstract setupRoutes(
    callMicroservice: CallMicroserviceFunction,
    handleServiceError: HandleServiceErrorFunction
  ): Router;

  /**
   * Gets the Express router instance
   */
  public getRouter(): Router {
    return this.router;
  }

  /**
   * Gets route information for documentation
   */
  public getRouteInfo(): { basePath: string; endpoints: string[] } {
    return {
      basePath: this.routeConfig.basePath,
      endpoints: this.routeConfig.endpoints.map(
        endpoint => `${endpoint.method} ${this.routeConfig.basePath}${endpoint.path}`
      )
    };
  }
}
