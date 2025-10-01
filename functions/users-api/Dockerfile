FROM fedora:40
WORKDIR /home/app

# Install Node.js
RUN dnf install -y nodejs npm && dnf clean all

# Install function dependencies
COPY package.json ./
RUN npm install --production

# Copy function code
COPY . .

# Set up the of-watchdog
USER root
ARG TARGETPLATFORM
ARG BUILDPLATFORM

# Install of-watchdog with proper architecture detection
ENV WATCHDOG_VERSION "0.10.11"
RUN ARCH=$(uname -m) && \
    if [ "$ARCH" = "x86_64" ]; then ARCH="x86_64"; fi && \
    if [ "$ARCH" = "aarch64" ]; then ARCH="arm64"; fi && \
    curl -sL https://github.com/openfaas/of-watchdog/releases/download/${WATCHDOG_VERSION}/of-watchdog-${ARCH} -o /usr/bin/fwatchdog && \
    chmod +x /usr/bin/fwatchdog

# Create app user (Fedora syntax)
RUN groupadd --system app && useradd --system --gid app app
USER app

# Health check
HEALTHCHECK --interval=3s CMD [ -e /tmp/.lock ] || exit 1

# Set environment for HTTP mode
ENV mode="http"
ENV upstream_url="http://127.0.0.1:3000"
ENV exec_timeout="10s"
ENV write_timeout="15s"
ENV read_timeout="15s"
ENV fprocess="node index.js"

# Create simple HTTP server
COPY <<EOF index.js
const handler = require("./handler");
const http = require("http");

const server = http.createServer(async (req, res) => {
    let body = "";
    req.on("data", chunk => body += chunk);
    req.on("end", async () => {
        try {
            const event = JSON.parse(body || "{}");
            const result = await handler(event, {
                status: (code) => ({
                    headers: (h) => ({
                        succeed: (data) => {
                            res.writeHead(code, {"Content-Type": "application/json", ...h});
                            res.end(JSON.stringify(data));
                        }
                    })
                })
            });
        } catch (e) {
            res.writeHead(500, {"Content-Type": "application/json"});
            res.end(JSON.stringify({error: e.message}));
        }
    });
});

server.listen(3000, () => console.log("Server running on port 3000"));
EOF

# Start watchdog
CMD ["fwatchdog"]
