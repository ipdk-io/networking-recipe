from system_tools.log import logging
from system_tools.services import OPIService

platforms_service = OPIService(1)
host_platform = self.platforms_service.get_host_platform()
result = self.host_platform.run_performance_fio(1)
logging.ptf_info(result)
