### Learning about distributing shards, buck, grpc and etc!
The constraint solver works through iterative local search:
1. Perturb current arrangement by shifting shards around (prioritizing moving unassigned shards to containers).
2. Check if new arrangement is better according to constraints.
3. Repeat a few thousand times. 

### Reads
https://research.facebook.com/publications/shard-manager-a-generic-shard-management-framework-for-geo-distributed-applications/
https://research.google/pubs/slicer-auto-sharding-for-datacenter-applications/
https://engineering.linkedin.com/apache-helix/apache-helix-framework-distributed-system-development
https://www.uber.com/blog/ringpop-open-source-nodejs-library/

### Setup
```bash setup.sh```
