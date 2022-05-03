## Get current trace id

```php
/**
 * @var string $traceId
 */
$traceId = skywalking_trace_id();
```

## Report logging to oap server

```php
/**
 * @var string $message
 * @var string $level
 */
skywalking_logging_report("log ...", "info")
```
