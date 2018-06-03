The three major steps in DATA TO COMPUTE

1. file hits landing zone


2. landing zone rule
   - checks extension
   - determines target (for compute) from resource metadata 
   - puts file to target resource 
   - (change of plan from Friday; no longer attaching job params as metadata)
     now going to spawn rules from command line ; can't pass params through
     the rule mechanism as this is prevented by ongoing remote-exec issue

3. hpc rule - schedules hpc job(s)
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
