fn main() -> Result<(), Box<dyn std::error::Error>> {
    tonic_build::configure()
        .build_server(false)
        .type_attribute(".", "#[derive(::serde::Deserialize)]")
        // .type_attribute(".", "#[serde(default)]")
        .compile(
            &[
                "./src/protocol/language-agent/Tracing.proto",
                "./src/protocol/management/Management.proto",
            ],
            &["./src/protocol/"],
        )?;
    Ok(())
}