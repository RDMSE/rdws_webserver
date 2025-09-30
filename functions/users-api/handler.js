'use strict'

const { exec } = require('child_process');
const util = require('util');
const execAsync = util.promisify(exec);

module.exports = async (event, context) => {
    const method = event.method || 'GET';
    const path = event.path || '/users';

    try {
        // C++ executable path for users service
        const executablePath = '/home/rdias/cpp-rest-server/services/users_service';

        // Call the C++ executable with method and path
        const command = `${executablePath} "${method}" "${path}"`;

        console.log(`Calling C++ service: ${command}`);

        const { stdout, stderr } = await execAsync(command, {
            timeout: 5000, // 5 seconds timeout
            encoding: 'utf8'
        });

        if (stderr) {
            console.error('C++ service stderr:', stderr);
        }

        // Parse JSON returned by C++ executable
        let responseData;
        try {
            responseData = JSON.parse(stdout.trim());
        } catch (parseError) {
            console.error('Failed to parse C++ response:', stdout);
            responseData = {
                error: "Invalid response from C++ service",
                rawResponse: stdout,
                serverless: true
            };
        }

        // Add metadata for serverless function
        responseData.serverlessWrapper = true;
        responseData.functionName = "users-api";
        responseData.executedCommand = command;
        responseData.timestamp = new Date().toISOString();

        return context
            .status(200)
            .headers({ "Content-Type": "application/json" })
            .succeed(responseData);

    } catch (error) {
        console.error('Error calling C++ service:', error);

        return context
            .status(500)
            .headers({ "Content-Type": "application/json" })
            .succeed({
                error: "Failed to execute C++ service",
                details: error.message,
                method: method,
                path: path,
                serverless: true,
                functionName: "users-api",
                timestamp: new Date().toISOString()
            });
    }
}
