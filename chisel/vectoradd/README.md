# Verilog generation

To generate verilog for this example, run:
````code
sbt run
````

This should produce:
````
HldAcceleratorWrapper.anno  HldAcceleratorWrapper.fir  HldAcceleratorWrapper.v
````

Change to the hld subdirectory and follow the instructions there to generate an ASE simulation model and run it.

# Testing 

To run the chisel tests, as always, run:
````code
sbt test
````
