--------------------------------------------------------------------------------
--    This file is owned and controlled by Xilinx and must be used solely     --
--    for design, simulation, implementation and creation of design files     --
--    limited to Xilinx devices or technologies. Use with non-Xilinx          --
--    devices or technologies is expressly prohibited and immediately         --
--    terminates your license.                                                --
--                                                                            --
--    XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" SOLELY    --
--    FOR USE IN DEVELOPING PROGRAMS AND SOLUTIONS FOR XILINX DEVICES.  BY    --
--    PROVIDING THIS DESIGN, CODE, OR INFORMATION AS ONE POSSIBLE             --
--    IMPLEMENTATION OF THIS FEATURE, APPLICATION OR STANDARD, XILINX IS      --
--    MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION IS FREE FROM ANY      --
--    CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE FOR OBTAINING ANY       --
--    RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY       --
--    DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE   --
--    IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR          --
--    REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF         --
--    INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A   --
--    PARTICULAR PURPOSE.                                                     --
--                                                                            --
--    Xilinx products are not intended for use in life support appliances,    --
--    devices, or systems.  Use in such applications are expressly            --
--    prohibited.                                                             --
--                                                                            --
--    (c) Copyright 1995-2014 Xilinx, Inc.                                    --
--    All rights reserved.                                                    --
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
--    Generated from core with identifier: xilinx.com:ip:cordic:4.0           --
--                                                                            --
--    The Xilinx CORDIC LogiCORE is a module for generation of the            --
--    generalized coordinate rotational digital computer (CORDIC) algorithm   --
--    which iteratively solves trigonometric, hyperbolic and square root      --
--    equations. The core is fully synchronous using a single clock.          --
--    Options include parameterizable data width and control signals. The     --
--    core supports either serial architecture for minimal area               --
--    implementations, or parallel architecture for speed optimization. The   --
--    core is delivered through the Xilinx CORE Generator System and          --
--    integrates seamlessly with the Xilinx design flow.                      --
--------------------------------------------------------------------------------

-- The following code must appear in the VHDL architecture header:

------------- Begin Cut here for COMPONENT Declaration ------ COMP_TAG
COMPONENT Square_Root
  PORT (
    x_in : IN STD_LOGIC_VECTOR(25 DOWNTO 0);
    nd : IN STD_LOGIC;
    x_out : OUT STD_LOGIC_VECTOR(13 DOWNTO 0);
    rdy : OUT STD_LOGIC;
    clk : IN STD_LOGIC
  );
END COMPONENT;
-- COMP_TAG_END ------ End COMPONENT Declaration ------------

-- The following code must appear in the VHDL architecture
-- body. Substitute your own instance name and net names.

------------- Begin Cut here for INSTANTIATION Template ----- INST_TAG
your_instance_name : Square_Root
  PORT MAP (
    x_in => x_in,
    nd => nd,
    x_out => x_out,
    rdy => rdy,
    clk => clk
  );
-- INST_TAG_END ------ End INSTANTIATION Template ------------

-- You must compile the wrapper file Square_Root.vhd when simulating
-- the core, Square_Root. When compiling the wrapper file, be sure to
-- reference the XilinxCoreLib VHDL simulation library. For detailed
-- instructions, please refer to the "CORE Generator Help".

