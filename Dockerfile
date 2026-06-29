# Build environment: official ps2dev toolchain + GNU make (not bundled upstream).
FROM ps2dev/ps2dev:latest
RUN apk add --no-cache make
WORKDIR /src
