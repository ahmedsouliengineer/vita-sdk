1. Core Principles

    Context over Content: A log without a TraceID is an orphan. All telemetry must be correlated.

    Vendor Agnostic: All instrumentation must use the OpenTelemetry API. No vendor-specific libraries allowed in the utils repo.

    Performance First: Use BatchLogRecordProcessor and BatchSpanProcessor. Instrumentation must never block the "hot path" of audio processing or LLM inference.

2. Naming Conventions & Hierarchy

2.1 Service Names

All services must be identified using the pattern: vita.<domain>.<service-name>

    STT Service: vita.ai.stt

    LLM Service: vita.ai.llm

    Mobile Gateway: vita.client.mobile

2.2 Attribute Namespacing

Never use generic keys like id or time. Always prefix with the vita. namespace to avoid collisions with system attributes.

    ✅ Good: vita.stt.confidence_score, vita.llm.token_count

    ❌ Bad: confidence, tokens

3. Distributed Tracing Standards

3.1 Trace Structure

Every user interaction (audio clip) is a Single Trace.

    Root Span: Created at the entry point (e.g., Mobile Mic Start).

    Child Spans: Each microservice (STT, LLM) creates a child span representing its specific work.

    Context Propagation: Services must propagate the traceparent header (W3C standard) via gRPC or HTTP headers.

3.2 AI-Specific Span Attributes (LLM)

Every LLM span must include these attributes for performance tracking:
| Attribute | Description | Example |
| :--- | :--- | :--- |
| gen_ai.request.model | The specific model name. | gpt-4o or llama-3-8b |
| gen_ai.usage.input_tokens | Tokens sent in the prompt. | 54 |
| gen_ai.usage.output_tokens | Tokens generated in response. | 120 |
| vita.llm.ttft_ms | Time to First Token (in milliseconds). | 180 |

4. Logging Standards

4.1 Severity Mapping

Level	Use Case
FATAL	System-level failure (e.g., GPU/CUDA crash, Disk Full).
ERROR	Request failure (e.g., LLM Timeout, STT Engine Error).
WARN	High latency (>3s), low STT confidence score (<0.4).
INFO	Transaction milestones (e.g., "Audio Received", "Response Sent").
DEBUG	Raw transcription text, full JSON payloads.

4.2 Log Decoration

The vita utils repo must automatically inject these into every log record:

    trace_id and span_id.

    service.name and service.version.

    deployment.environment (e.g., edge, cloud, dev).

5. Metrics Standards (The "Golden Signals")

Every service must export these metrics at 60-second intervals:

    Latency (Histogram): vita.<service>.duration (seconds).

    Throughput (Counter): vita.<service>.requests_total.

    Errors (Counter): vita.<service>.errors_total (with error.type attribute).

    Hardware (Gauge): system.cpu.utilization, system.memory.usage.

6. Environment Profiles

The utils repo must adjust behavior based on the VITA_ENV variable:

🚀 Cloud Profile (VITA_ENV=cloud)

    Exporter: OTLP / gRPC.

    Batch Size: 512 records.

    Export Interval: 5 seconds.

🛰️ Edge/Hardware Profile (VITA_ENV=edge)

    Exporter: OTLP / HTTP (JSON).

    Persistence: Enable local disk queuing (via OTel Collector).

    Sampling: 100% for errors; 25% for successful traces to save edge bandwidth.

7. Operational Requirements

    Clean Shutdown: The CleanUp() function in log.h, trace.h, and metric.h must call ForceFlush() with a 5-second timeout to prevent data loss.

    Resilience: Telemetry is a "side-car" concern. If the OTel Collector is unreachable, the Vita application must continue to process audio and text without interruption.


Vita_sdk/
├── include/
│   └── vita/
│       ├── log.hpp         # Macros & high-level API (Standard 4.1)
│       ├── trace.hpp       # Span & Context API (Standard 3.1)
│       ├── metric.hpp      # Golden Signal API (Standard 5)
│       ├── telemetry.hpp   # Resource/Namespacing (Standard 2.1 & 2.2)
│       └── config.hpp      # Profile/Env Engine (Standard 6)
└── src/
    ├── log_setup.cpp       # LoggerProvider & Batch Processor
    ├── trace_setup.cpp     # TracerProvider & W3C Propagator
    ├── metric_setup.cpp    # MeterProvider & 60s Interval
    └── common_setup.cpp    # Shared Resource & Profile loading logic