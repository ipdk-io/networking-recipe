from datetime import datetime


def passed(message):
    now = datetime.now()
    timestamp = "%d/%d/%d,%s:%s:%02d" % (
        now.month,
        now.day,
        now.year,
        now.hour,
        now.minute,
        now.second,
    )
    print(f"{timestamp}  PASSED: {message}")


def failed(message):
    now = datetime.now()
    timestamp = "%d/%d/%d,%s:%s:%02d" % (
        now.month,
        now.day,
        now.year,
        now.hour,
        now.minute,
        now.second,
    )
    print(f"{timestamp}  FAILED: {message}")


def warned(message):
    now = datetime.now()
    timestamp = "%d/%d/%d,%s:%s:%02d" % (
        now.month,
        now.day,
        now.year,
        now.hour,
        now.minute,
        now.second,
    )
    print(f"{timestamp}  WARNED: {message}")


def info(message):
    now = datetime.now()
    timestamp = "%d/%d/%d,%s:%s:%02d" % (
        now.month,
        now.day,
        now.year,
        now.hour,
        now.minute,
        now.second,
    )
    print(f"{timestamp}  INFO: {message}")
