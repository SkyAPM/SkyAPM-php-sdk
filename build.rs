fn main() -> Result<(), Box<dyn std::error::Error>> {
    tonic_build::configure().compile(
        &[
            "./src/protocol/language-agent/Tracing.proto",
            "./src/protocol/management/Management.proto",
        ],
        &["./src/protocol/"],
    )?;
    Ok(())
}