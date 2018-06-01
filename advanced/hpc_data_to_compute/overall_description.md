The three major steps in DATA TO COMPUTE

1. file hits landing zone


2. landing zone rule
  - checks extension
  - determines target (for compute) from resource metadata 
  - puts file to target resource 
  - attach metadata describing HPC tasks (thumbnail sizes)

3. hpc rule
  - schedules hpc job(s)
     - prolog
     - epilog
        * registers products into catalog
        * determines target (for LTS) from resource metadata
        * replicates products to target resource 
        * trims original products from HPC filesystem
      
Finally
---
all steps are manual in this exercise, but could be chained 
into an automated process
