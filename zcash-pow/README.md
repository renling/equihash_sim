# Python reference implementation of the Zcash proof-of-work

The PoW currently being used by Zcash is Equihash, a memory-hard algorithm
based on the Generalised Birthday Problem.

## all Requirements removed

##* `cryptography`
##* `pyblake2`
##* `progressbar2` (optional for progress bars in `-v` and `-vv` modes)

## Demo miner

To run:

```python
./pow.py
./pow.py -h
```
