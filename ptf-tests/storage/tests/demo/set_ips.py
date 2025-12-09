from system_tools.services import OPIService

platforms_service = OPIService()
lp_platform = platforms_service.get_lp_platform()
lp_platform.imc_device.login()
lp_platform.acc_device.login()
lp_platform.set_internal_ips()
