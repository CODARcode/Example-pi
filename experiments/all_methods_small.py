"""
"""

from codar.cheetah import Experiment
from codar.cheetah import parameters as p

class SmallPiExperiment(Experiment):
    name = "pi-all-methods-small"
    # TODO: in future could support multiple executables if needed, with
    # the idea that they have same input/output/params, but are compiled
    # with different options. Could be modeled as p.ParamExecutable.
    app_exe = "pi-gmp"
    supported_machines = ['titan', 'local']

    runs = [
     p.SchedulerGroup(nodes=1,
      parameter_groups=
      [p.ParameterGroup([
        p.ParamCmdLineArg("method", 1, ["mc", "trap"]),
        p.ParamCmdLineArg("precision", 2, [64, 128, 256]),
        p.ParamCmdLineArg("iterations", 3, [10, 100, 1000,
                                            10_000, 100_000, 1_000_000,
                                            10_000_000]),
        ]),
       p.ParameterGroup([
        p.ParamCmdLineArg("method", 1, ["atan", "atan2"]),
        p.ParamCmdLineArg("precision", 2, [2**i for i in range(6, 19)]),
        p.ParamCmdLineArg("iterations", 3, [10, 100, 1000, 10_000,
                                            100_000, 1_000_000]),
        ]),
      ]),
    ]
