# Actor-based-Multicore-Architecture-Simulator

It's a functional and timing hardware simulator based on SystemC, used to evaluate our actor-based multicore architecture.

Here is a brief introduction of our architecture:

Distributed training of ultra-large-scale AI models poses challenges to the communication capabilities and scalability of chip architectures. Wafer-level chips achieve ultra-high computing density and communication performance by integrating a large number of computing cores and interconnection networks on the same wafer, making them ideal for training ultra-large-scale AI models. AMCoDA is a many-core dataflow hardware architecture based on the Actor model, which aims to realize distributed training of AI models on wafer-level chips by taking advantage of the high concurrency, asynchronous message passing and high scalability of the Actor parallel programming model. The design of AMCoDA includes three levels: compu-ting model, execution model and hardware architecture. Experiments show that AMCoDA can widely support various parallel modes and collective communications in distributed training, and complete the deployment and execution of complex distributed training strategies flexibly and efficiently.
