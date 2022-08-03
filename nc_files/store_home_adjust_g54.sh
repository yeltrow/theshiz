#
#  How to use this script.
#  1. Home the machine.  
#  2. Jog the machine to get to 0,0,0 on the handwheels.
#  3. Save the G28 position to memory.
#  G28.1
#  5. Set the tool to tool 0.  
#  6. Touch off the X to -0.1
#  7. Touch off the Y to -0.1
#  8. Change to tool 12.
#  9. Verify the tool offsets have changed. (Z)
# 10. Jog over and touch the top of the fixture plate as Z -1.077
# 11. Jog the tool to the tool change position.
# 12. Issue a G28 to move to the stored position.
# 13. Change to tool 0
# 14. Issue G43 to clear tool length offsets.
# 15. Record the G54 relative position in a screenshot.
# 16. Enter the new values in this script.
# 17. Put the edge finder back in and check the new offsets.  
# 18. If everything is correct, this script can be ran after
#     jogging the handwheels to 0 and the fixture plate offsets in
#     G54 will be correct.  
#     G28 can be used to verify 0,0,0 and the offsets in the screenshot
#     at any time.
# Set the current tool to 0, no tool.
axis-remote --mdi "G92.1"
axis-remote --mdi "M61 Q0"
# Set the tool length offset to active.
# makes sure we are using 0,0,0 for tool length offset.
axis-remote --mdi "G43"
# Save the current position as our G28 saved position.  Helpful
# for being able to go back and manually check the zeros
axis-remote --mdi "G28.1"
# Set the G54 work offset (P1) to the measured position of the 
# homing sequence in G54.  
#axis-remote --mdi "G10 L20 P1 X-0.245 Y11.3655 Z1.6154"
#2022-03-16
axis-remote --mdi "G54"
axis-remote --mdi "M61 Q0"
axis-remote --mdi "G43"
#axis-remote --mdi "G10 L20 P1 X-0.0330 Y11.3664 Z1.7125"
axis-remote --mdi "G10 L20 P1 X-0.0330 Y11.3664 Z1.6166"

 

