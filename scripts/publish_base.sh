#!/bin/bash
set -e

# Configuration
IMAGE_NAME="ghcr.io/ahmedsouliengineer/vita-base"
VERSION=${1:-latest}

# I am choosing to check for the token before starting
if [ -z "$CR_PAT" ]; then
    echo "❌ Error: CR_PAT environment variable is not set."
    echo "Please run: export CR_PAT=your_token_here"
    exit 1
fi

echo "🔐 Logging into GHCR..."
echo $CR_PAT | docker login ghcr.io -u ahmedsouliengineer --password-stdin

echo "🚀 Starting build for $IMAGE_NAME:$VERSION..."
docker build -t $IMAGE_NAME:$VERSION -f .devcontainer/Dockerfile.base .

if [ "$VERSION" != "latest" ]; then
    echo "🏷️  Tagging $VERSION as latest..."
    docker tag $IMAGE_NAME:$VERSION $IMAGE_NAME:latest
fi

echo "📤 Pushing to GHCR..."
docker push $IMAGE_NAME:$VERSION
if [ "$VERSION" != "latest" ]; then
    docker push $IMAGE_NAME:latest
fi

echo "✅ Successfully published $IMAGE_NAME:$VERSION"