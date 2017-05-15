# Introduction

We designed a real-time pitch shifter on the DE1-SoC FPGA that was controlled by the ARM core using a GUI. The motivation for this project stemmed from the fact that the large amount of processing power available on the FPGA is well suited to audio input and output streaming, along with the required intermediate processing. As a result, we decided to utilize these capabilities to design and implement a real-time pitch shifter that could perform the following tasks: pitch-shift the left and right audio outputs independently with manual pitch tuning, produce voice chords using the right and left audio outputs along with the original voice, and produce time-variant pitch shifting by modulating the appropriate parameters at varying rates. The final product is able to produce many different voice effects, all while being controlled from a simple, user-friendly GUI displayed on a VGA monitor.








## Welcome to GitHub Pages

You can use the [editor on GitHub](https://github.com/jmt329/PitchShifter/edit/master/README.md) to maintain and preview the content for your website in Markdown files.

Whenever you commit to this repository, GitHub Pages will run [Jekyll](https://jekyllrb.com/) to rebuild the pages in your site, from the content in your Markdown files.

### Markdown

Markdown is a lightweight and easy-to-use syntax for styling your writing. It includes conventions for

```markdown
Syntax highlighted code block

# Header 1
## Header 2
### Header 3

- Bulleted
- List

1. Numbered
2. List

**Bold** and _Italic_ and `Code` text

[Link](url) and ![Image](src)
```

For more details see [GitHub Flavored Markdown](https://guides.github.com/features/mastering-markdown/).

### Jekyll Themes

Your Pages site will use the layout and styles from the Jekyll theme you have selected in your [repository settings](https://github.com/jmt329/PitchShifter/settings). The name of this theme is saved in the Jekyll `_config.yml` configuration file.

### Support or Contact

Having trouble with Pages? Check out our [documentation](https://help.github.com/categories/github-pages-basics/) or [contact support](https://github.com/contact) and we’ll help you sort it out.
