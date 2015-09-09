import matplotlib.pyplot as plt


# The slices will be ordered and plotted counter-clockwise.
labels = 'S', 'N', 'PS'
sizes = [10,43,16]
colors = ['yellowgreen', 'gold', 'lightskyblue']
explode = (0, 0, 0) # only "explode" the 2nd slice (i.e. 'Hogs')

patches, texts, autotexts = plt.pie(sizes, explode=explode, labels=labels, colors=colors, shadow=True, autopct='%1.1f%%')
# Set aspect ratio to be equal so that pie is drawn as a circle.
texts[0].set_fontsize(30)
texts[1].set_fontsize(30)
texts[2].set_fontsize(30)
autotexts[0].set_fontsize(20)
autotexts[1].set_fontsize(20)
autotexts[2].set_fontsize(20)
plt.axis('equal')

plt.show()