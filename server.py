
import json
import subprocess
from pathlib import Path
from http.server import HTTPServer, SimpleHTTPRequestHandler


BASE_DIR = Path(__file__).resolve().parent
BACKEND = BASE_DIR / "gui_backend.exe"
GUI_FILE = BASE_DIR / "MEMORYX_GUI.html"


class MemoryXHandler(SimpleHTTPRequestHandler):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(BASE_DIR), **kwargs)

    def do_GET(self):
        if self.path == "/":
            self.path = "/MEMORYX_GUI.html"

        return super().do_GET()

    def send_json(self, status, data):
        response = json.dumps(data).encode("utf-8")

        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(response)))
        self.end_headers()
        self.wfile.write(response)

    def do_POST(self):

        try:
            length = int(self.headers.get("Content-Length", 0))
            request = json.loads(self.rfile.read(length))

            allowed_workloads = [
                "Basic",
                "Fragmentation",
                "Mixed",
                "Saved"
            ]

            # =====================================
            # SAVE CUSTOM WORKLOAD ENDPOINT
            # =====================================

            if self.path == "/api/save-workload":

                workload_name = request.get("name")
                processes = request.get("processes")
                history = request.get("history", [])

                # Validate workload name
                if (
                    not isinstance(workload_name, str)
                    or not workload_name.strip()
                    or "\n" in workload_name
                    or "\r" in workload_name
                ):
                    raise ValueError("Invalid workload name")

                # Validate processes list
                if (
                    not isinstance(processes, list)
                    or len(processes) == 0
                    or len(processes) > 1000
                ):
                    raise ValueError(
                        "Enter between 1 and 1000 processes"
                    )

                # Validate history list
                if not isinstance(history, list):
                    raise ValueError("Invalid event history")

                if len(history) > 10000:
                    raise ValueError("Too many events")

                # =================================
                # VALIDATE PROCESSES
                # =================================

                used_ids = set()
                validated_processes = []

                for process in processes:

                    if not isinstance(process, dict):
                        raise ValueError("Invalid process data")

                    fields = [
                        "pid",
                        "memory",
                        "arrival",
                        "execution",
                        "priority"
                    ]

                    if any(
                        isinstance(process.get(field), bool)
                        or not isinstance(process.get(field), int)
                        for field in fields
                    ):
                        raise ValueError(
                            "Process values must be integers"
                        )

                    pid = process["pid"]
                    memory = process["memory"]
                    arrival = process["arrival"]
                    execution = process["execution"]
                    priority = process["priority"]

                    if (
                        pid <= 0
                        or memory <= 0
                        or arrival < 0
                        or execution <= 0
                        or priority <= 0
                    ):
                        raise ValueError("Invalid process values")

                    if pid in used_ids:
                        raise ValueError("Duplicate Process ID")

                    used_ids.add(pid)

                    validated_processes.append(
                        (pid, memory, arrival, execution, priority)
                    )

                # =================================
                # VALIDATE EVENT HISTORY
                # =================================

                for event in history:

                    if not isinstance(event, dict):
                        raise ValueError("Invalid event")

                    action = event.get("action")

                    if action not in (
                        "Allocation",
                        "Deallocation"
                    ):
                        raise ValueError("Invalid event action")

                    pid = event.get("pid")

                    if (
                        isinstance(pid, bool)
                        or not isinstance(pid, int)
                        or pid not in used_ids
                    ):
                        raise ValueError("Invalid event PID")

                    if action == "Allocation":

                        memory = event.get("memory")

                        if (
                            isinstance(memory, bool)
                            or not isinstance(memory, int)
                            or memory <= 0
                        ):
                            raise ValueError(
                                "Invalid allocation memory"
                            )

                # =================================
                # SAVE WORKLOAD FILE
                # =================================

                workload_file = BASE_DIR / "memoryx_workload.txt"
                workload_temp = BASE_DIR / "memoryx_workload.tmp"

                with workload_temp.open(
                    "w",
                    encoding="utf-8"
                ) as file:

                    file.write("MEMORYX_WORKLOAD_V1\n")
                    file.write(workload_name.strip() + "\n")
                    file.write(str(len(validated_processes)) + "\n")

                    for process in validated_processes:
                        file.write(
                            "{} {} {} {} {}\n".format(*process)
                        )

                workload_temp.replace(workload_file)

                # =================================
                # SAVE JSON EVENT HISTORY
                # =================================

                history_file = BASE_DIR / "memoryx_history.json"
                history_temp = BASE_DIR / "memoryx_history.json.tmp"

                with history_temp.open(
                    "w",
                    encoding="utf-8"
                ) as file:

                    json.dump(history, file, indent=4)

                history_temp.replace(history_file)

                # =================================
                # SAVE TEXT EVENT HISTORY FOR C++
                # =================================

                history_text_file = BASE_DIR / "memoryx_history.txt"
                history_text_temp = BASE_DIR / "memoryx_history.txt.tmp"

                with history_text_temp.open(
                    "w",
                    encoding="utf-8"
                ) as file:

                    file.write("MEMORYX_HISTORY_V1\n")
                    file.write(str(len(history)) + "\n")

                    for event in history:

                        action = event["action"]
                        pid = event["pid"]
                        memory = event.get("memory", 0)

                        file.write(
                            f"{action} {pid} {memory}\n"
                        )

                history_text_temp.replace(history_text_file)

                # =================================
                # SUCCESS RESPONSE
                # =================================

                self.send_json(200, {
                    "success": True,
                    "message": (
                        "Workload saved successfully: "
                        + workload_name.strip()
                    )
                })

                return

            # =====================================
            # WORKLOAD ENDPOINT
            # =====================================

            if self.path == "/api/workload":

                workload_type = request.get("type")

                if workload_type not in allowed_workloads:
                    raise ValueError("Invalid workload type")

                command = [
                    str(BACKEND),
                    workload_type
                ]

            # =====================================
            # MEMORY SIMULATION ENDPOINT
            # =====================================

            elif self.path == "/api/simulation":

                workload_type = request.get("type")
                total_memory = request.get("totalMemory")
                strategy = request.get("strategy")

                allowed_strategies = [
                    "First Fit",
                    "Best Fit",
                    "Worst Fit"
                ]

                if workload_type not in allowed_workloads:
                    raise ValueError("Invalid workload type")

                if strategy not in allowed_strategies:
                    raise ValueError("Invalid allocation strategy")

                if (
                    isinstance(total_memory, bool)
                    or not isinstance(total_memory, int)
                    or total_memory <= 0
                ):
                    raise ValueError("Invalid total memory")

                command = [
                    str(BACKEND),
                    workload_type,
                    str(total_memory),
                    strategy
                ]

            # =====================================
            # STRATEGY COMPARISON ENDPOINT
            # =====================================

            elif self.path == "/api/comparison":

                workload_type = request.get("type")
                total_memory = request.get("totalMemory")

                if workload_type not in allowed_workloads:
                    raise ValueError("Invalid workload type")

                if (
                    isinstance(total_memory, bool)
                    or not isinstance(total_memory, int)
                    or total_memory <= 0
                ):
                    raise ValueError("Invalid total memory")

                command = [
                    str(BACKEND),
                    workload_type,
                    str(total_memory),
                    "COMPARE"
                ]

            # =====================================
            # ADAPTIVE ENGINE ENDPOINT
            # =====================================

            elif self.path == "/api/adaptive":

                workload_type = request.get("type")
                total_memory = request.get("totalMemory")

                if workload_type not in allowed_workloads:
                    raise ValueError("Invalid workload type")

                if (
                    isinstance(total_memory, bool)
                    or not isinstance(total_memory, int)
                    or total_memory <= 0
                ):
                    raise ValueError("Invalid total memory")

                command = [
                    str(BACKEND),
                    workload_type,
                    str(total_memory),
                    "ADAPTIVE"
                ]

            # =====================================
            # CPU SCHEDULING ENDPOINT
            # =====================================

            elif self.path == "/api/cpu-scheduling":

                workload_type = request.get("type")
                total_memory = request.get("totalMemory")

                if workload_type not in allowed_workloads:
                    raise ValueError("Invalid workload type")

                if (
                    isinstance(total_memory, bool)
                    or not isinstance(total_memory, int)
                    or total_memory <= 0
                ):
                    raise ValueError("Invalid total memory")

                command = [
                    str(BACKEND),
                    workload_type,
                    str(total_memory),
                    "CPU"
                ]

            else:

                self.send_json(404, {
                    "success": False,
                    "message": "Endpoint not found"
                })

                return

            # =====================================
            # RUN C++ BACKEND
            # =====================================

            result = subprocess.run(
                command,
                cwd=str(BASE_DIR),
                capture_output=True,
                text=True,
                timeout=10
            )

            if result.returncode != 0:
                raise RuntimeError(
                    result.stderr.strip()
                    or result.stdout.strip()
                    or "C++ backend failed"
                )

            response = json.loads(result.stdout)

            if not response.get("success"):
                self.send_json(400, response)
                return

            self.send_json(200, response)

        except Exception as error:

            self.send_json(400, {
                "success": False,
                "message": str(error)
            })


# =====================================
# CHECK REQUIRED FILES
# =====================================

if not BACKEND.exists():
    raise FileNotFoundError("gui_backend.exe not found")

if not GUI_FILE.exists():
    raise FileNotFoundError("MEMORYX_GUI.html not found")


# =====================================
# START SERVER
# =====================================

server = HTTPServer(
    ("127.0.0.1", 8000),
    MemoryXHandler
)

print("MEMORYX server running at http://127.0.0.1:8000")
print("Press Ctrl+C to stop the server.")

server.serve_forever()
