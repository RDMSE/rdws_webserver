/**
 * AWS Lambda-like Event and Context types for C++ microservices integration
 */

export interface HttpRequestInfo {
  method: string;
  path: string;
  resource: string;
  body: string;
  headers: Record<string, string>;
  queryStringParameters: Record<string, string>;
  pathParameters: Record<string, string>;
}

export interface RequestContext {
  requestId: string;
  stage: string;
  httpMethod: string;
  resourcePath: string;
  protocol: string;
  sourceIp: string;
  userAgent: string;
  requestTimeEpoch: number;
}

export interface LambdaEvent {
  httpMethod: string;
  path: string;
  resource: string;
  body: string;
  headers: Record<string, string>;
  queryStringParameters: Record<string, string>;
  pathParameters: Record<string, string>;
  requestContext: RequestContext;
  stageVariables: Record<string, string>;
}

export interface LambdaContext {
  requestId: string;
  functionName: string;
  functionVersion: string;
  timeoutMs: number;
  memoryLimitMB: number;
}

/**
 * Helper function to create LambdaEvent from Express Request
 */
export function createLambdaEvent(
  req: any, // Express.Request
  requestId: string,
  pathParameters: Record<string, string> = {}
): LambdaEvent {
  return {
    httpMethod: req.method,
    path: req.path,
    resource: req.route?.path || req.path,
    body: req.body ? JSON.stringify(req.body) : '',
    headers: req.headers || {},
    queryStringParameters: req.query || {},
    pathParameters,
    requestContext: {
      requestId,
      stage: process.env.NODE_ENV || 'development',
      httpMethod: req.method,
      resourcePath: req.route?.path || req.path,
      protocol: 'HTTP/1.1',
      sourceIp: req.ip || req.connection.remoteAddress || '127.0.0.1',
      userAgent: req.get('User-Agent') || 'unknown',
      requestTimeEpoch: Date.now(),
    },
    stageVariables: {},
  };
}

/**
 * Helper function to create LambdaContext
 */
export function createLambdaContext(
  requestId: string,
  functionName: string,
  functionVersion: string = '1.0',
  timeoutMs: number = 30000,
  memoryLimitMB: number = 128
): LambdaContext {
  return {
    requestId,
    functionName,
    functionVersion,
    timeoutMs,
    memoryLimitMB,
  };
}