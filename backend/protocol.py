import json


def encode_message(message):
    """Convert a Python dictionary into newline-delimited JSON."""
    return (json.dumps(message) + "\n").encode("utf-8")


def decode_message(data):
    """Convert JSON data into a Python dictionary."""
    return json.loads(data)
