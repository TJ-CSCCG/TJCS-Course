
The design files are located at
D:/Digilent/Projects/Nexys4_DDR/Nexys4DdrUserDemoSrc/ipcore_dir:

   - ddr.veo:
        veo template file containing code that can be used as a model
        for instantiating a CORE Generator module in a HDL design.

   - ddr.xco:
       CORE Generator input file containing the parameters used to
       regenerate a core.

   - ddr_flist.txt:
        Text file listing all of the output files produced when a customized
        core was generated in the CORE Generator.

   - ddr_readme.txt:
        Text file indicating the files generated and how they are used.

   - ddr_xmdf.tcl:
        ISE Project Navigator interface file. ISE uses this file to determine
        how the files output by CORE Generator for the core can be integrated
        into your ISE project.

   - ddr.csv:
        Includes the pin out information which is used as support file
        for PlanAhead.

   - ddr.gise and ddr.xise:
        ISE Project Navigator support files. These are generated files and
        should not be edited directly.

   - ddr directory.

In the ddr directory, three folders are created:
   - docs:
        This folder contains MIG user guide.

   - example_design:
        This folder includes script files to implement and simulate the design.
        This includes the traffic generator RTL modules and example_top module.

   - user_design:
        This folder includes the all RTL modules of controller, phy and
        user interface RTL modules. UCF file is provided.

The example_design and user_design folders contain several other folders
and files. All these output folders are discussed in more detail in
MIG user guide (ug586_7Series_MIS.pdf) located in docs folder.
    