FROM python:3.13.3-alpine
WORKDIR /app
RUN apk add --no-cache \
    build-base \
    g++ \
    make \
    && pip install --upgrade pip
ENV PYTHONUNBUFFERED=1
COPY requirements.txt .
RUN pip install -r requirements.txt
COPY . .
RUN make build/main
RUN chmod +x build/main && rm -rf .dockerignore build/obj Dockerfile include Makefile requirements.txt src/*.cpp
CMD ["python", "src/python/app.py"]