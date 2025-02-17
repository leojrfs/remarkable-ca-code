import json
from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route("/report", methods=["POST"])
def report():
    try:
        print("request.user_agent=" + str(request.user_agent))
        print("request.data=" + str(request.data))
        # Parse JSON request
        data = request.get_json()
        if data is None:
            return jsonify({"error": "Invalid JSON"}), 400

        # Validation: Ensure required keys exist
        required_fields = ["hostname", "uptime", "memory", "disk"]
        missing_fields = [field for field in required_fields if field not in data]

        if missing_fields:
            return (
                jsonify({"error": f"Missing fields: {', '.join(missing_fields)}"}),
                400,
            )

        # validate field types
        if not isinstance(data["hostname"], str) or not isinstance(data["uptime"], int):
            return jsonify({"error": "Invalid field types"}), 400

        print("jsonifyed_data=\n" + json.dumps(data, indent=4, sort_keys=True))

        return jsonify({"message": "Report received"}), 201

    except Exception as e:
        return jsonify({"error": str(e)}), 400


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8090, debug=True)
