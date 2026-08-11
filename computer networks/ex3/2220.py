class Solution(object):
    def minBitFlips(self, start, goal):
        xor_result = start ^ goal
        count = 0
        while xor_result:
            xor_result &= xor_result - 1
            count += 1
        return count