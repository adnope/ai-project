#!/bin/bash

# Create virtual environment
python3 -m venv env

# Activate virtual environment
source env/bin/activate

# Install requirements
pip install -r requirements.txt



echo "Setup complete! Run 'source env/bin/activate' to activate the virtual environment"
