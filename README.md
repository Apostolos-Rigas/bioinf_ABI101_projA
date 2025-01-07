# bioinf_ABI101_projA

## Table of Contents
- [Description](#description)
- [Usage](#usage)
- [Side Functionality](#side_functionality)
- [Theoretical Foundations](#theoretical_foundations)
- [Known Bugs](#known_bugs)
- [License](#license)
- [Authors](#authors)

## Description
This project constitutes an assignement for the course UNIX and Programming Principles (UNIX και Αρχές Προγραμματισμού - ΕΒΠ101) at Democritus University of Thrace, offered during the winter semester of the Master's Program in Bioinformatics (https://bioinfo.mbg.duth.gr/).

## Usage
With this C program, the user can input as many RNA sequences they desire, and the program analyzes and prints to the screen (or any output device specified) the segments of the sequence that correspond to bacterial gene regions.

Initially, the user must define the maximum sequence length they intend to input. If they wish to input sequences longer than the initial maximum length, they must return to the main menu and redefine the maximum sequence length.

Examples of using the app from terminal:
*For already compiled .c file:
  1) cd {path_where_compiled_exe_is_located}
  2) ./{file_name e.g. bioinf_projA} stdout ARCHIVE_FILE.txt
*To compile the app and run it:
  2) make
  3) ./bioinf_projA stdout ARCHIVE_FILE.txt

Warning! The length of the sequence must be a multiple of the codons length (default value is 3).

## Side_Functionality
- At the end of the analysis session, the results are saved in an archive file, which can either be provided by the user as a terminal parameter or is taken as the default "./ARCHIVE_FILE.txt".
- The output of the program can also be defined by the user as a terminal parameter. This allows for interactive use of the program utilizing the screen (default mode) or non-interactive use by redirecting the output to a file (e.g. for system administrators or for logging and later preview from a remote terminal).

## Theoretical_Foundations
The analysis of the sequence and its validation as a coding sequence is based on the following concepts from cell biology and genetics:

1. **Consecutive Stop Codons Without a Start Codon Between**: It is possible for bacterial genomes to contain consecutive stop codons without an intervening start codon, often seen in genes with overlapping reading frames, where one gene's stop codon serves as the start codon for another gene. Overlapping genes are common in compact genomes and may be an evolutionary strategy to maximize genetic information (Benoit et al., 2012) ([Benoit et al., 2012](https://biologydirect.biomedcentral.com/articles/10.1186/1745-6150-7-30)).

2. **Consecutive Start Codons Without a Stop Codon Between**: Although rare, it is theoretically possible for bacterial sequences to have consecutive start codons without a stop codon in between. Typically, genes have their own start and stop codons, and while genes can overlap, they do so in a way that ensures proper translation initiation and termination (Chan et al., 2017) ([Chan et al., 2017](https://biology.stackexchange.com/questions/46427/multiple-start-and-stop-codons-in-mrna-and-pre-mrna)).

3. **Sequence Length Less Than 9**: Genes that encode functional proteins are generally longer than 9 nucleotides, as this length is too short to encode a meaningful protein. However, some small peptides or regulatory elements are shorter. Small open reading frames (sORFs) are examples of such sequences, often involved in regulatory functions (Sánchez et al., 2019) ([Sánchez et al., 2019](https://pmc.ncbi.nlm.nih.gov/articles/PMC7256928/)).

4. **Nested Genes**: Nested genes, where one gene is located entirely within the coding sequence of another, are relatively rare in bacteria but do occur. These genes typically have their own promoters and regulatory elements, allowing them to be transcribed and translated independently. This phenomenon is observed in both prokaryotic and eukaryotic genomes, though it is more common in complex organisms (Eichler et al., 2015) ([Eichler et al., 2015](https://www.nature.com/articles/srep13634)).

5. **Encoding a Gene in Both Directions**: Bacterial genomes can encode genes in both directions, referred to as convergent or divergent genes. This arrangement allows for efficient use of genomic space and can regulate gene expression. Genes encoded in both directions are often co-regulated, contributing to bacterial adaptability and efficiency (Hwang et al., 2020) ([Hwang et al., 2020](https://arxiv.org/abs/2008.10758)).

* The papers mentioned where retrieved from the Web using ChatGPT and then assessed for credibility by the project's authors.

## Known_Bugs
- If -in any menu- user presses the arrows (up/down/left/right) and then types an input, the app stucks in an infinite loop.
- If it happens to exit the programm unexpectedly (e.g. killing the terminal process) while in a colored background, the color of the terminal background remains colored (but if one re-runs the app and then exit properly the color is fixed/reset to default terminal color).
- If sequence is longer than the pre-specified (by the user) max length, app terminates due to segmentation fault (11).
- There are very rare incidents when the position of the valid coding sequence is off by one codon.

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details. Library cJSON used in this project is also covered by MIT License.

## Authors
- Rigas Apostolos-Nikolaos [@username](https://github.com/Apostolos-Rigas)

Copyright (c) 2025 Niklaus
