from scipy.optimize import fsolve
import numpy as np
from matplotlib import pyplot as plt

# Transistor characteristics
# Q1 is input transistor
# Q2 is output transistor
Vt: float = 26e-3  # thermal voltage
Alpha = 0.99  # alpha of Q2
Is = 1e-12  # base-emitter reverse bias current of Q2

# Circuit characteristics
Vee = -9  # negative rail
Ree = 4700  # tail resistor
Vb2 = 0  # base voltage of Q2

# Model data
n = 1000  # The number of data points
Vin_low = -20 * Vt
Vin_high = 20 * Vt


def func(ie, v_in):
    """ The set of equations which define the circuits behaviour.
    Calculates an error vector for the ie[0] and ie[1] estimates.
    ie[0] os the emitter current of Q1, ie[1] of Q2."""
    # Emitter voltage
    ve = Vee + (ie[0] + ie[1]) * Ree
    # Shockley equation for Q1
    z1 = ie[0] - Is / Alpha * (np.exp((v_in - ve) / Vt) - 1)
    # Shockley equation for Q2
    z2 = ie[1] - Is / Alpha * (np.exp((Vb2 - ve) / Vt) - 1)
    return [z1, z2]


# Input array in (A), returns an array in (mA)
def m_amps(i):
    return i * 1000


# Input array in (V), returns an array in (VT)
def v_thermal(v):
    return v / Vt


# Input voltage
Vin = np.linspace(Vin_low, Vin_high, n)

# Emitter currents
# ie[0] is emitter current for input transistor
# ie[1] is emitter current for output transistor
ie = np.empty((n, 2))

# Find initial guesses for the root
# Check the output when running this script. If fsolve() does
# not approach solutions soon enough, try better initial guesses!
guess = (-0.6 - Vee) / Ree / 2
root = np.array([guess, guess])

# Try to solve the system for each v_in. For the next v_in, use the
# solution of the previous v_in as an initial guess.
for i, v_in in enumerate(Vin):
    root = fsolve(func, root, args=v_in)
    ie[i] = root

# Calculate emitter and base-emitter voltage from emitter currents.
iee = ie[:, 0] + ie[:, 1]  # total tail current
ve = iee * Ree + Vee
vbe1 = Vin - ve

# Plot it all
# Display input voltage in [Vt], currents in [mA]
ax1 = plt.subplot(211)
plt.plot(Vin / Vt, m_amps(ie[:, 0]), 'g-', label='ie1')
plt.plot(Vin / Vt, m_amps(ie[:, 1]), 'b-', label='ie2')
plt.setp(ax1.get_xticklabels(), visible=False)
plt.legend()
plt.grid(True)
ax1.set_ylabel('ie1, ie2 [mA]')

ax2 = plt.subplot(212, sharex=ax1)
plt.plot(v_thermal(Vin), ve, 'r-', label='Ve')
plt.plot(v_thermal(Vin), vbe1, 'y-', label='Vbe1')
plt.legend()
plt.grid(True)
ax2.set_xlabel('Input voltage [Vt]')
ax2.set_ylabel('Ve, Vbe1 [V]')

fig = plt.gcf()
fig.canvas.set_window_title('IF diffpair')

plt.show()
