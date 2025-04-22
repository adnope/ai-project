FROM python:3.10-slim

WORKDIR /app

RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    && rm -rf /var/lib/apt/lists/*

ENV PYTHONUNBUFFERED=1
ENV PORT=8080

COPY requirements.txt .
RUN pip install --upgrade pip
RUN pip install -r requirements.txt

COPY . .

RUN make

RUN chmod +x main

EXPOSE $PORT

# CMD uvicorn app:app --host 0.0.0.0 --port $PORT

# CMD ["python", "app.py"]

CMD ["./main -b"]