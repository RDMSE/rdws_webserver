/**
 * Shared Types for API Gateway
 *
 * Common interfaces and types used across all microservice routes
 */

import { Request, Response } from 'express';

// Configuration interface
export interface Config {
  port: number;
  buildPath: string;
  environment: string;
  timeout: number;
}

// Service response from microservices
export interface ServiceResponse {
  [key: string]: any;
  gateway?: {
    requestId: string;
    serviceName: string;
    executionTime: number;
    timestamp: string;
  };
}

// Error response format
export interface ErrorResponse {
  error: true;
  message: string;
  requestId: string;
  timestamp?: string;
  availableRoutes?: string[];
}

// Health check interfaces
export interface HealthService {
  status: 'healthy' | 'unhealthy';
  responseTime?: number;
  error?: string;
}

export interface HealthResponse {
  status: 'ok' | 'degraded';
  timestamp: string;
  requestId: string;
  config: {
    environment: string;
    buildPath: string;
    timeout: number;
  };
  services: Record<string, HealthService>;
}

// API Documentation interface
export interface ApiDocsResponse {
  title: string;
  version: string;
  description: string;
  endpoints: Record<string, string>;
  requestId: string;
}

// Extended Request interface
declare global {
  namespace Express {
    interface Request {
      requestId: string;
    }
  }
}

// Microservice call function type
export type CallMicroserviceFunction = (
  serviceName: string,
  method: string,
  path: string,
  requestId: string
) => Promise<ServiceResponse>;

// Error handler function type
export type HandleServiceErrorFunction = (error: Error, req: Request, res: Response) => void;

// Route configuration for each microservice
export interface RouteConfig {
  serviceName: string;
  basePath: string;
  endpoints: {
    path: string;
    method: 'GET' | 'POST' | 'PUT' | 'DELETE';
    handler: string;
    validation?: (req: Request) => string | null; // Returns error message or null
  }[];
}

// Microservice router interface
export interface MicroserviceRouter {
  routeConfig: RouteConfig;
  setupRoutes: (
    callMicroservice: CallMicroserviceFunction,
    handleServiceError: HandleServiceErrorFunction
  ) => any; // Express Router
  getRouteInfo: () => { basePath: string; endpoints: string[] };
}
