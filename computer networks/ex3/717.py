class Solution(object):
    def isOneBitCharacter(self, bits):
        """
        :type bits: List[int]
        :rtype: bool
        """
        i = 0
        n = len(bits)
        
        # Traverse the array up to the second-to-last element
        while i < n - 1:
            if bits[i] == 1:
                i += 2  # 2-bit character (10 or 11) takes 2 steps
            else:
                i += 1  # 1-bit character (0) takes 1 step
                
        # If loop ends exactly at the last element, it must be a 1-bit character
        return i == n - 1