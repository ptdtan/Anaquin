# Anaquin

**Anaquin** is a C++/R bioinformatics framework for quantitative controls in next-generation sequencing experiments. 

Respository for the R-package is hosted by Bioconductor and available at: https://github.com/Bioconductor-mirror/Anaquin.

The project is maintained by <b>Ted Wong</b>, bioinformatics software engineer at Garvan Institute of Medical Research.

This is a beta software as we are trying to work with the bioinformatics community. Please send us your suggestions (eg. what do you want Anaquin to do?). Detailed workflow guide is avaialble for download at www.sequin.xyz.

## License

<a href='https://opensource.org/licenses/BSD-3-Clause'>The BSD 3-Clause License</a>

## Citation

* Spliced synthetic genes as internal controls in RNA sequencing experiments
* Representing genetic variation with synthetic DNA standards

## Bioinformatics

<ul>
<li>RNA-Seq</li>
<li>Genetic variation</li>
<li>Metagenomcis</li>
</ul>

## Overview

The project was started in 2015 by <a href='https://www.google.com.au/url?sa=t&rct=j&q=&esrc=s&source=web&cd=1&cad=rja&uact=8&ved=0CB4QFjAAahUKEwiWt5b-7p3IAhWEjJQKHcxhDMg&url=http%3A%2F%2Fwww.garvan.org.au%2F&usg=AFQjCNF03pFvjJsIYqEbmxMV3SBTC5PJxg&sig2=jxHlEHfy_CNSJ4cZyVfvVQ'>Garvan Institute of Medical Research</a>. See <a href='http://www.sequin.xyz'>www.sequin.xyz</a> for further details. In particular, the <a href='www.sequin.xyz/about/introduction/'>introduction page</a> should be helpful.

<img src='http://www.anaquin.org/wp-content/uploads/2015/07/About_Intro_F1_3.svg'>

Next-generation sequencing (NGS) enables rapid, cheap and high-throughput determination of DNA (or RNA) sequences within a user’s sample. NGS methods have been applied widely, and have fuelled major advances in the life sciences and clinical health care over the past decade. However, NGS typically generates a large amount of sequencing data that must be first analyzed and interpreted with bioinformatic tools. There is no standard way to perform an analysis of NGS data; different tools provide different advantages in different situations. For example, the tools for finding small deletions in the human genome are different to the tools to find large deletions. The sheer number, complexity and variation of sequences further compound this problem, and there is little reference by which compare next-generation sequencing and analysis.

To address this problem, we have developed a suite of synthetic nucleic-acid standards that we term sequins. Sequins represent genetic features, such as genes, large structural rearrangements, that are often analyzed with NGS. However, whilst sequins may act like a natural genetic feature, their primary sequence is artificial, with no extended homology to natural genetic sequences. Sequins are fractionally added to the extracted nucleic-acid sample prior to library preparation, so they are sequenced along with your sample of interest. The reads that derive from sequins can be identified by their artificial sequences that prevent their cross-alignment to the genome of known organisms. 

Due to their ability to model real genetic features, sequins can act as internal qualitative controls for a wide range of NGS applications. To date, we have developed sequencing that model gene expression and alternative splicing, fusion genes, small and large structural variation between human genomes, immune receptors, microbe communities, mutations in mendelian diseases and cancer, and we even undertake custom designs according to client’s specific requirements.

By combining sequins at different concentrations to from a mixture, we can also establish quantitative ladders sequins by which to measure all types of quantitative events in genome biology. For example by varying the concentration of RNA sequins we can emulate changes in gene expression or alternative splicing, or by varying relative DNA sequin abundance we can emulate heterozygous genotypes by modulating variant sequins.

Finally, to aid in the analysis of sequins, we have also developed a software toolkit we call <b>Anaquin</b>. This contains a wide range of tools for some of the most common analysis or problems that use sequins. This includes quality control and troubleshooting steps in your NGS pipeline, providing quantitative measurements of sequence libraries, or assess third-party bioinformatic software. However, this toolkit is simply a starting point to a huge range of statistical analysis made possible by sequins.

## Dependencies

<ul>
<li> <a href='http://www.boost.org/users/download/'>Boost</a>
<li> <a href='http://eigen.tuxfamily.org'>Eigen</a>
<li> <a href='https://github.com/attractivechaos/klib'>Klib</a>
</ul>

The source code requires a C++11 compiler to build.
