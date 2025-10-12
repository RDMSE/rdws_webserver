#!/usr/bin/env node

/**
 * Local API Gateway Test Client
 *
 * This is a command-line tool to test the API Gateway locally.
 * It allows you to make HTTP requests to the gateway and see responses.
 *
 * Usage:
 *   npm run local
 *   npm run local -- get /users
 *   npm run local -- post /users '{"name":"John","email":"john@example.com"}'
 *   npm run local -- health
 */

import { program } from 'commander';
import axios, { AxiosResponse } from 'axios';
import { startServer, app } from '../../api-gateway';
import { Server } from 'http';

// Configuration
const LOCAL_PORT = 8080;
const BASE_URL = `http://localhost:${LOCAL_PORT}`;

// Colors for console output
const colors = {
  green: '\x1b[32m',
  red: '\x1b[31m',
  yellow: '\x1b[33m',
  blue: '\x1b[34m',
  magenta: '\x1b[35m',
  cyan: '\x1b[36m',
  white: '\x1b[37m',
  reset: '\x1b[0m',
};

function colorize(color: keyof typeof colors, text: string): string {
  return `${colors[color]}${text}${colors.reset}`;
}

function formatResponse(response: AxiosResponse): void {
  console.log(`\n${colorize('cyan', '='.repeat(60))}`);
  console.log(`${colorize('green', 'Status:')} ${response.status} ${response.statusText}`);
  console.log(`${colorize('yellow', 'Headers:')}`);
  Object.entries(response.headers).forEach(([key, value]) => {
    console.log(`  ${colorize('blue', key)}: ${value}`);
  });
  console.log(`${colorize('magenta', 'Response:')}`);
  console.log(JSON.stringify(response.data, null, 2));
  console.log(`${colorize('cyan', '='.repeat(60))}\n`);
}

function formatError(error: any): void {
  console.log(`\n${colorize('red', '='.repeat(60))}`);
  console.log(`${colorize('red', 'ERROR:')}`);

  if (error.response) {
    console.log(
      `${colorize('red', 'Status:')} ${error.response.status} ${error.response.statusText}`
    );
    console.log(`${colorize('red', 'Response:')}`);
    console.log(JSON.stringify(error.response.data, null, 2));
  } else if (error.request) {
    console.log(`${colorize('red', 'Request failed:')} No response received`);
    console.log(error.message);
  } else {
    console.log(`${colorize('red', 'Error:')} ${error.message}`);
  }
  console.log(`${colorize('red', '='.repeat(60))}\n`);
}

async function makeRequest(method: string, endpoint: string, data?: string): Promise<void> {
  try {
    const url = endpoint.startsWith('http') ? endpoint : `${BASE_URL}${endpoint}`;
    const requestData = data ? JSON.parse(data) : undefined;

    console.log(`${colorize('blue', 'Request:')} ${method.toUpperCase()} ${url}`);
    if (requestData) {
      console.log(`${colorize('blue', 'Data:')} ${JSON.stringify(requestData, null, 2)}`);
    }

    const response = await axios({
      method: method.toLowerCase(),
      url,
      data: requestData,
      headers: {
        'Content-Type': 'application/json',
      },
      timeout: 10000,
    });

    formatResponse(response);
  } catch (error) {
    formatError(error);
    process.exit(1);
  }
}

async function startLocalServer(): Promise<Server> {
  console.log(`${colorize('green', 'Starting local API Gateway...')}`);

  return new Promise((resolve, reject) => {
    const server = startServer();

    server.on('listening', () => {
      console.log(`${colorize('green', 'API Gateway started successfully!')}`);
      console.log(`${colorize('cyan', `Server running at: ${BASE_URL}`)}`);
      console.log(`${colorize('yellow', 'Available endpoints:')}`);
      console.log(`   GET  ${BASE_URL}/health`);
      console.log(`   GET  ${BASE_URL}/api-docs`);
      console.log(`   GET  ${BASE_URL}/users`);
      console.log(`   GET  ${BASE_URL}/users/:id`);
      console.log(`   GET  ${BASE_URL}/orders`);
      console.log(`   GET  ${BASE_URL}/orders/:id`);
      console.log(`   GET  ${BASE_URL}/users/:userId/orders`);
      console.log('');
      resolve(server);
    });

    server.on('error', (error: any) => {
      if (error.code === 'EADDRINUSE') {
        console.log(
          `${colorize(
            'yellow',
            `Port ${LOCAL_PORT} is already in use. Assuming server is running...`
          )}`
        );
        resolve(server);
      } else {
        reject(error);
      }
    });
  });
}

async function healthCheck(): Promise<void> {
  try {
    console.log(`${colorize('blue', 'Performing health check...')}`);
    await makeRequest('get', '/health');
  } catch (error) {
    console.log(`${colorize('red', 'Health check failed! Make sure the API Gateway is running.')}`);
    process.exit(1);
  }
}

async function interactiveMode(): Promise<void> {
  console.log(`${colorize('green', 'Interactive mode started!')}`);
  console.log(`${colorize('yellow', 'Available commands:')}`);
  console.log('  health              - Check API Gateway health');
  console.log('  docs                - Get API documentation');
  console.log('  users               - List all users');
  console.log('  users <id>          - Get user by ID');
  console.log('  orders              - List all orders');
  console.log('  orders <id>         - Get order by ID');
  console.log('  users <id> orders   - Get orders for user');
  console.log('  exit                - Exit interactive mode');
  console.log('');

  const readline = require('readline');
  const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
  });

  const askQuestion = (): Promise<string> => {
    return new Promise(resolve => {
      rl.question(`${colorize('cyan', 'local-api>')} `, resolve);
    });
  };

  while (true) {
    try {
      const input = (await askQuestion()).trim();

      if (input === 'exit') {
        break;
      }

      const parts = input.split(' ');
      const command = parts[0];

      switch (command) {
        case 'health':
          await makeRequest('get', '/health');
          break;
        case 'docs':
          await makeRequest('get', '/api-docs');
          break;
        case 'users':
          if (parts.length === 1) {
            await makeRequest('get', '/users');
          } else if (parts.length === 2) {
            await makeRequest('get', `/users/${parts[1]}`);
          } else if (parts.length === 3 && parts[2] === 'orders') {
            await makeRequest('get', `/users/${parts[1]}/orders`);
          } else {
            console.log(`${colorize('red', 'Invalid users command')}`);
          }
          break;
        case 'orders':
          if (parts.length === 1) {
            await makeRequest('get', '/orders');
          } else if (parts.length === 2) {
            await makeRequest('get', `/orders/${parts[1]}`);
          } else {
            console.log(`${colorize('red', 'Invalid orders command')}`);
          }
          break;
        case '':
          // Empty command, just continue
          break;
        default:
          console.log(`${colorize('red', 'Unknown command:')} ${command}`);
          break;
      }
    } catch (error) {
      console.log(`${colorize('red', 'Error in interactive mode:')} ${error}`);
    }
  }

  rl.close();
}

// CLI Setup
program.name('local-api').description('Local API Gateway Test Client').version('1.0.0');

program
  .command('start')
  .description('Start the API Gateway server')
  .action(async () => {
    try {
      await startLocalServer();
      console.log(`${colorize('green', 'Server started successfully!')}`);
      console.log(`${colorize('yellow', 'Press Ctrl+C to stop the server')}`);

      // Keep the process alive
      process.on('SIGINT', () => {
        console.log(`\n${colorize('yellow', 'Stopping server...')}`);
        process.exit(0);
      });

      // Prevent the process from exiting
      setInterval(() => {}, 1000);
    } catch (error) {
      console.error(`${colorize('red', 'Failed to start server:')} ${error}`);
      process.exit(1);
    }
  });

program.command('health').description('Check API Gateway health').action(healthCheck);

program
  .command('get <endpoint>')
  .description('Make a GET request to the API Gateway')
  .action(async (endpoint: string) => {
    await makeRequest('get', endpoint);
  });

program
  .command('post <endpoint> [data]')
  .description('Make a POST request to the API Gateway')
  .action(async (endpoint: string, data?: string) => {
    await makeRequest('post', endpoint, data);
  });

program
  .command('put <endpoint> [data]')
  .description('Make a PUT request to the API Gateway')
  .action(async (endpoint: string, data?: string) => {
    await makeRequest('put', endpoint, data);
  });

program
  .command('delete <endpoint>')
  .description('Make a DELETE request to the API Gateway')
  .action(async (endpoint: string) => {
    await makeRequest('delete', endpoint);
  });

program
  .command('interactive')
  .alias('i')
  .description('Start interactive mode')
  .action(async () => {
    try {
      await startLocalServer();
      await interactiveMode();
    } catch (error) {
      console.error(`${colorize('red', 'Failed to start interactive mode:')} ${error}`);
      process.exit(1);
    }
  });

// Default action (no command)
program.action(async () => {
  try {
    await startLocalServer();
    await interactiveMode();
  } catch (error) {
    console.error(`${colorize('red', 'Failed to start:')} ${error}`);
    process.exit(1);
  }
});

// Parse command line arguments
program.parse();
