# Performance gains after Moore’s law ends

The figure "Performance gains after Moore’s law ends" is making a stack-level argument about where future speedups will 
come from once transistor scaling stops providing the steady, broadly applicable performance improvements that the industry 
relied on for decades. It divides the world into "the Bottom" and "the Top." The Bottom stands for advances in semiconductor 
technology, smaller, faster, cheaper transistors, and the automatic performance gains that used to follow. The Top stands for 
everything above raw device scaling: software, algorithms, and hardware architecture. The figure’s core claim is that, 
as Moore’s-law-style scaling slows, the main remaining headroom for performance shifts upward into these top layers,
reversing the historical pattern where manufacturing improvements lifted nearly all boats with comparatively little
effort from programmers and system designers.


In the Top portion, the figure lays out three "technologies" and, for each, an "opportunity" and example levers. 
Under Software, the opportunity is software performance engineering. The figure is pointing out that large performance 
improvements will increasingly depend on deliberate engineering work: reducing overhead that has accumulated in modern 
software stacks and making programs exploit the actual performance features of contemporary machines. "Removing software bloat" 
refers to cutting unnecessary abstraction costs and runtime overhead that creep in through general-purpose frameworks, 
excessive layering, avoidable copying and allocation, and other forms of extra work that do not contribute to the program’s output.
"Tailoring software to hardware features" refers to shaping code and data so that it benefits from modern architectural
realities such as many cores, SIMD and vector units, deep cache hierarchies, NUMA effects, and accelerators. 
The figure’s implication is that hardware will not automatically speed up legacy software in the way it once did; 
software must increasingly be adapted so it can actually use the parallelism and locality the hardware offers.


Under Algorithms, the opportunity is new algorithms. The message here is that algorithmic changes can still deliver very
large speedups, often far larger than micro-optimizations, because they can reduce the amount of work the computer must do
or reduce the amount of costly data movement. The examples "new problem domains" and "new machine models" hint at two ways 
this happens. As new application areas grow, they often begin with relatively unrefined methods, so algorithmic innovation can
produce dramatic improvements. Separately, "new machine models" emphasizes that the classical way of analyzing algorithms, 
often assuming a simple serial machine with uniform memory access, does not match modern reality, where memory traffic and 
parallel coordination dominate. Designing algorithms with models that reflect caches, bandwidth limits, latency, and parallel 
execution can lead to methods that scale better and waste less time and energy on communication.


Under Hardware architecture, the opportunity is hardware streamlining, with examples of processor simplification and domain 
specialization. The point is that, when you cannot count on frequency scaling and ever-more-complex general-purpose cores to 
raise single-thread speed, one path forward is to simplify cores so they are more energy- and area-efficient, and then use the 
saved resources to deliver throughput by replication or by improving the memory system. Another path is specialization: building 
hardware tailored to a particular workload class rather than trying to be equally good at everything. Special-purpose units can 
remove general-purpose overhead and devote silicon to the operations that matter most in that domain, producing large gains 
in performance per watt and performance per area. This architectural shift aligns with the broader thesis: instead of the 
Bottom delivering uniform gains, the Top increasingly finds performance by matching hardware capabilities and design to what 
software and algorithms actually need.


Taken as a whole, the figure is arguing that the post-Moore era will not be defined by a single, predictable, universal 
scaling mechanism. Performance gains will come from a portfolio of improvements across software engineering, algorithmic 
breakthroughs, and architectural choices, and those gains will be less automatic and more workload-dependent. In other words,
the easy, uniform dividend from semiconductor scaling diminishes, and progress relies more on human-driven design choices 
throughout the computing stack.