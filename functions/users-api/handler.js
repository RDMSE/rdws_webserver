'use strict'

const { exec } = require('child_process');
const util = require('util');
const execAsync = util.promisify(exec);

module.exports = async (event, context) => {
    const method = event.method || 'GET';
    const path = event.path || '/users';

    try {
        // Caminho para o executável C++ do serviço de usuários
        const executablePath = '/home/rdias/cpp-rest-server/services/users_service';

        // Chamar o executável C++ passando método e path
        const command = `${executablePath} "${method}" "${path}"`;

        console.log(`Calling C++ service: ${command}`);

        const { stdout, stderr } = await execAsync(command, {
            timeout: 5000, // 5 segundos de timeout
            encoding: 'utf8'
        });

        if (stderr) {
            console.error('C++ service stderr:', stderr);
        }

        // Parse do JSON retornado pelo executável C++
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

        // Adicionar metadados da função serverless
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
