from datetime import datetime
import logging


def ptf_info(msg, *args, **kwargs):
    now = datetime.now()
    timestamp = "%d/%d/%d,%s:%s:%02d" % (
        now.month,
        now.day,
        now.year,
        now.hour,
        now.minute,
        now.second,
    )
    print(f"{timestamp}  INFO: {msg}")
    logging.info(msg, *args, **kwargs)


logging.ptf_info = ptf_info
